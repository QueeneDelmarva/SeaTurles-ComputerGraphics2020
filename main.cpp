#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <chrono>

using namespace tinyobj;


struct tinyObj
{
    attrib_t                attrib;
    std::vector<shape_t>    shapes;
    std::vector<material_t> materials;
};



static
bool read_tiny_obj(const char* path, tinyObj* o)
{
    std::string err;
    std::string warn;
    return LoadObj(&o->attrib, &o->shapes, &o->materials, &warn, &err, path, 0, false);
}


static
void check(bool c, const char* m)
{
    if (!c)
        printf("CHECK FAILED : %s\n", m);
}

#define CHECK(_c) check(_c, #_c)


static
void compare_mesh(fastObjMesh* m, tinyObj* o)
{
    CHECK(m->group_count == o->shapes.size());

    for (unsigned int ii = 0; ii < m->group_count; ii++)
    {
        const fastObjGroup& grp = m->groups[ii];
        const shape_t& shp = o->shapes[ii];

        std::string grp_name;
        if (grp.name)
            grp_name = std::string(grp.name);

        CHECK(shp.name == grp_name);
        CHECK(shp.mesh.num_face_vertices.size() == grp.face_count);

        int idx = 0;
        for (unsigned int jj = 0; jj < grp.face_count; jj++)
        {
            unsigned int fv = m->face_vertices[grp.face_offset + jj];

            CHECK(shp.mesh.num_face_vertices[jj] == fv);

            for (unsigned int kk = 0; kk < fv; kk++)
            {
                index_t oi = shp.mesh.indices[idx];
                fastObjIndex mi = m->indices[grp.index_offset + idx];

                CHECK(oi.vertex_index + 1 == mi.p);
                CHECK(oi.texcoord_index + 1 == mi.t);
                CHECK(oi.normal_index + 1 == mi.n);

                if (mi.p)
                {
                    CHECK(o->attrib.vertices[3 * oi.vertex_index + 0] == m->positions[3 * mi.p + 0]);
                    CHECK(o->attrib.vertices[3 * oi.vertex_index + 1] == m->positions[3 * mi.p + 1]);
                    CHECK(o->attrib.vertices[3 * oi.vertex_index + 2] == m->positions[3 * mi.p + 2]);
                }

                if (mi.t)
                {
                    CHECK(o->attrib.texcoords[2 * oi.texcoord_index + 0] == m->texcoords[2 * mi.t + 0]);
                    CHECK(o->attrib.texcoords[2 * oi.texcoord_index + 1] == m->texcoords[2 * mi.t + 1]);
                }

                if (mi.n)
                {
                    CHECK(o->attrib.normals[3 * oi.normal_index + 0] == m->normals[3 * mi.n + 0]);
                    CHECK(o->attrib.normals[3 * oi.normal_index + 1] == m->normals[3 * mi.n + 1]);
                    CHECK(o->attrib.normals[3 * oi.normal_index + 2] == m->normals[3 * mi.n + 2]);
                }

                idx++;
            }
        }
        printf("Result: %d number", idx);
    }
}


int main(int argc, const char* argv[])
{
    /*if (argc != 2)
    {
        printf("%s <obj file>\n", argv[0]);
        return -1;
    }*/

    printf("Reading with fast_obj\n");

    auto fast_start = std::chrono::high_resolution_clock::now();

    fastObjMesh* m = fast_obj_read("/Users/queene/Desktop/Sea Turtle/Sea Turtle/Sea Turtle/ModelsTurtle/SeaTurtle3DMoving.obj");

    auto fast_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> fast_time = fast_end - fast_start;

    /*if (!m)
    {
        printf("Failed!\n");
        return -1;
    }
*/
    printf("Took %0.2f secs\n", fast_time.count());


    printf("Reading with tiny_obj_loader\n");
    tinyObj o;

    auto tiny_start = std::chrono::high_resolution_clock::now();

    bool success = read_tiny_obj("/Users/queene/Desktop/Sea Turtle/Sea Turtle/Sea Turtle/ModelsTurtle/SeaTurtle3DMoving.obj", &o);

    auto tiny_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tiny_time = tiny_end - tiny_start;

    if (!success)
    {
        printf("Failed!\n");
        return -1;
    }

    printf("Took %0.2f secs\n", tiny_time.count());

    printf("Checking...\n");
    compare_mesh(m, &o);

    printf(" of surface polygons of the Sea Turtle\n");



    fast_obj_destroy(m);

    return 0;
}
