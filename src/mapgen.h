#ifndef _MAPGEN_H_
#define _MAPGEN_H_
#include <map>
#include <string>
#include "mapgenformat.h"
#include "mapgen_functions.h"

//////////////////////////////////////////////////////////////////////////
///// function pointer class; provides absract referencing of
///// map generator functions written in multiple ways for per-terrain
///// random selection pool
enum mapgen_function_type {
    MAPGENFUNC_ERROR,
    MAPGENFUNC_C,
    MAPGENFUNC_LUA,
    MAPGENFUNC_JSON,
};

class mapgen_function {
    public:
    mapgen_function_type ftype;
    int weight;
    virtual void dummy_() = 0;
    virtual mapgen_function_type function_type() { return ftype;/*MAPGENFUNC_ERROR;*/ };
};


/////////////////////////////////////////////////////////////////////////////////
///// builtin mapgen
class mapgen_function_builtin : public virtual mapgen_function {
    public:
    building_gen_pointer fptr;
    mapgen_function_builtin(building_gen_pointer ptr, int w = 1000) : fptr(ptr) {
        ftype = MAPGENFUNC_C;
        weight = w;
    };
    mapgen_function_builtin(std::string sptr, int w = 1000);
    virtual void dummy_() {}
};

/////////////////////////////////////////////////////////////////////////////////
///// json mapgen (and friends)
struct jmapgen_int {
  short val;
  short valmax;
  jmapgen_int(int v) : val(v), valmax(v) {}
  jmapgen_int(int v, int v2) : val(v), valmax(v2) {}
  jmapgen_int( point p ) : val(p.x), valmax(p.y) {}
  
  int get() {
    return ( val == valmax ? val : rng(val, valmax) );
  }
};
/* todo
int place_items(items_location loc, const int chance, const int x1, const int y1,
                  const int x2, const int y2, bool ongrass, const int turn);

void add_spawn(std::string type, const int count, const int x, const int y, bool friendly = false,
                const int faction_id = -1, const int mission_id = -1,
                std::string name = "NONE");

*/
enum jmapgen_setmap_op {
    JMAPGEN_SETMAP_TER,
    JMAPGEN_SETMAP_FURN,
    JMAPGEN_SETMAP_TRAP,
    JMAPGEN_SETMAP_RADIATION
};

struct jmapgen_setmap_point {
    jmapgen_int x;
    jmapgen_int y;
    jmapgen_setmap_op op;
    jmapgen_int val;
    int chance;
    jmapgen_int repeat;
    jmapgen_setmap_point(
       jmapgen_int ix, jmapgen_int iy, jmapgen_setmap_op iop, jmapgen_int ival, 
       int ione_in = 1, jmapgen_int irepeat = jmapgen_int(1,1)
       ) : x(ix), y(iy), op(iop), val(ival), chance(ione_in), repeat(irepeat) {}
    bool apply( map * m );
};

struct jmapgen_spawn_item {
    jmapgen_int x;
    jmapgen_int y;
    std::string itype;
    jmapgen_int amount;
    int chance;
    jmapgen_int repeat;
    jmapgen_spawn_item(
        jmapgen_int ix, jmapgen_int iy, std::string iitype, jmapgen_int iamount, int ichance = 1, jmapgen_int irepeat = jmapgen_int(1,1) ) : 
        x(ix), y(iy), itype(iitype), amount(iamount), chance(ichance), repeat(irepeat) {}
    void apply( map * m ) {
        if ( chance == 1 || one_in( chance ) ) {
            const int trepeat = repeat.get() + 1;
            for (int i = 0; i <= trepeat; i++) {
                m->spawn_item( x.get(), y.get(), itype, amount.get() );
            }
        }
    }
};

class mapgen_function_json : public virtual mapgen_function {
    public:
    virtual void dummy_() {}
    void setup_setmap(JsonArray &parray);
    bool setup();
    void apply(map * m,oter_id id,mapgendata md ,int t,float d);
    mapgen_function_json(std::string s, int w = 1000) {
        ftype = MAPGENFUNC_JSON;
        weight = w;
        jdata = s;
        mapgensize = 24;
        fill_ter = -1;
        is_ready = false;
    }
    std::string jdata;
    int mapgensize;
    int fill_ter;
    terfurn_tile * format;
    std::vector<jmapgen_setmap_point> setmap_points;
    std::vector<jmapgen_spawn_item> spawnitems;

    terfurn_tile * tmpgrid; // for dupe checks etc
    bool is_ready;
};

/////////////////////////////////////////////////////////////////////////////////
///// lua mapgen
class mapgen_function_lua : public virtual mapgen_function {
    public:
    virtual void dummy_() {}
    const std::string scr;
    mapgen_function_lua(std::string s, int w = 1000) : scr(s) {
        ftype = MAPGENFUNC_LUA;
        weight = w;
        // scr = s; // todo; if ( luaL_loadstring(L, scr.c_str() ) ) { error }
    }
};
/////////////////////////////////////////////////////////
///// global per-terrain mapgen function lists
/*
 * Load mapgen function of any type from a jsonobject
 */
mapgen_function * load_mapgen_function(JsonObject &jio, const std::string id_base, int default_idx);
/*
 * Load the above directly from a file via init, as opposed to riders attached to overmap_terrain. Added check
 * for oter_mapgen / oter_mapgen_weights key, multiple possible ( ie, [ "house", "house_base" ] )
 */
void load_mapgen ( JsonObject &jio );
/*
 * stores function ref and/or required data
 */
extern std::map<std::string, std::vector<mapgen_function*> > oter_mapgen;
/*
 * random selector list for the nested vector above, as per indivdual mapgen_function_::weight value
 */
extern std::map<std::string, std::map<int, int> > oter_mapgen_weights;
/*
 * Sets the above after init, and initializes mapgen_function_json instances as well
 */
void calculate_mapgen_weights();

/// move to building_generation
enum room_type {
    room_null,
    room_closet,
    room_lobby,
    room_chemistry,
    room_teleport,
    room_goo,
    room_cloning,
    room_vivisect,
    room_bionics,
    room_dorm,
    room_living,
    room_bathroom,
    room_kitchen,
    room_bedroom,
    room_backyard,
    room_study,
    room_mine_shaft,
    room_mine_office,
    room_mine_storage,
    room_mine_fuel,
    room_mine_housing,
    room_bunker_bots,
    room_bunker_launcher,
    room_bunker_rifles,
    room_bunker_grenades,
    room_bunker_armor,
    room_mansion_courtyard,
    room_mansion_entry,
    room_mansion_bedroom,
    room_mansion_library,
    room_mansion_kitchen,
    room_mansion_dining,
    room_mansion_game,
    room_mansion_pool,
    room_mansion_study,
    room_mansion_bathroom,
    room_mansion_gallery,
    room_split
};

void house_room(map *m, room_type type, int x1, int y1, int x2, int y2);
// helpful functions
bool connects_to(oter_id there, int dir);
void mapgen_rotate( map * m, oter_id terrain_type, bool north_is_down = false );
// wrappers for map:: functions
void line(map *m, const ter_id type, int x1, int y1, int x2, int y2);
void line_furn(map *m, furn_id type, int x1, int y1, int x2, int y2);
void fill_background(map *m, ter_id type);
void fill_background(map *m, ter_id (*f)());
void square(map *m, ter_id type, int x1, int y1, int x2, int y2);
void square(map *m, ter_id (*f)(), int x1, int y1, int x2, int y2);
void square_furn(map *m, furn_id type, int x1, int y1, int x2, int y2);
void rough_circle(map *m, ter_id type, int x, int y, int rad);
void add_corpse(game *g, map *m, int x, int y);

#endif
