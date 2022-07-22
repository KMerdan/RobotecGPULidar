#include <Logger.h>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rgl/api/experimental.h>
#include <utils/testUtils.h>

using namespace ::testing;

class ApiExperimentalUnitTest : public ::testing::Test {
protected:
    rgl_lidar_t lidar = nullptr;
    rgl_mesh_t mesh = nullptr;
    rgl_entity_t entity = nullptr;

    std::vector<rgl_vec3f> vertices;
    std::vector<rgl_vec3i> indices;
    rgl_mat3x4f identity = {
        .value = {
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
        }
    };

    std::string readFile(std::filesystem::path path)
    {
        std::stringstream buffer;
        buffer << std::ifstream(path).rdbuf();
        return buffer.str();
    }
};

TEST_F(ApiExperimentalUnitTest, rgl_configure_logging)
{
    std::filesystem::path logFilePath { std::filesystem::temp_directory_path() / std::filesystem::path("RGL-log.txt") };

    ASSERT_THAT(logFilePath.c_str(), NotNull());
    EXPECT_RGL_SUCCESS(rgl_configure_logging(RGL_LOG_LEVEL_INFO, logFilePath.c_str(), true));
    ASSERT_THAT(std::filesystem::exists(logFilePath), IsTrue());
    Logger::instance().flush();
    ASSERT_THAT(readFile(logFilePath), HasSubstr("Logging configured"));

    RGL_TRACE("This is RGL trace log."); // Should be not printed
    RGL_INFO("This is RGL info log.");
    RGL_WARN("This is RGL warn log.");
    RGL_ERROR("This is RGL error log.");
    RGL_CRITICAL("This is RGL critical log.");
    Logger::instance().flush();

    ASSERT_THAT(readFile(logFilePath), Not(HasSubstr("trace")));
    ASSERT_THAT(readFile(logFilePath), HasSubstr("info"));
    ASSERT_THAT(readFile(logFilePath), HasSubstr("warn"));
    ASSERT_THAT(readFile(logFilePath), HasSubstr("error"));
    ASSERT_THAT(readFile(logFilePath), HasSubstr("critical"));
    EXPECT_RGL_SUCCESS(rgl_configure_logging(RGL_LOG_LEVEL_OFF, nullptr, false));
}

TEST(ApiExperimentalUnitTestVersion, rgl_get_version_info)
{
    int major, minor, patch;

    EXPECT_RGL_SUCCESS(rgl_get_version_info(&major, &minor, &patch));
    EXPECT_EQ(major, RGL_VERSION_MAJOR);
    EXPECT_EQ(minor, RGL_VERSION_MINOR);
    EXPECT_EQ(patch, RGL_VERSION_PATCH);

    EXPECT_RGL_INVALID_ARGUMENT(rgl_get_version_info(nullptr, &minor, &patch), "out_major != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_get_version_info(&major, nullptr, &patch), "out_minor != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_get_version_info(&major, &minor, nullptr), "out_patch != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_get_version_info(nullptr, nullptr, nullptr), "out_major != nullptr");
}

TEST_F(ApiExperimentalUnitTest, rgl_mesh_create)
{
    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_create(nullptr, vertices.data(), 0, indices.data(), 0), "mesh != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_create(&mesh, vertices.data(), 0, indices.data(), 0), "vertices != nullptr");

    vertices.push_back({ .value = { 1.0, 2.0, 3.0 } });

    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_create(&mesh, vertices.data(), 0, indices.data(), 0), "vertex_count > 0");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_create(&mesh, vertices.data(), vertices.size(), indices.data(), 0), "indices != nullptr");

    indices.push_back({ .value = { 1, 2, 3 } });

    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_create(&mesh, vertices.data(), vertices.size(), indices.data(), 0), "index_count > 0");
    EXPECT_RGL_SUCCESS(rgl_mesh_create(&mesh, vertices.data(), vertices.size(), indices.data(), indices.size()));

    EXPECT_RGL_SUCCESS(rgl_mesh_destroy(mesh));
    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_destroy(nullptr), "mesh != nullptr");
    // TODO(piotr.rybicki): Fix return error 500 RGL_INTERNAL_EXCEPTION
    // EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_destroy(mesh), "mesh != nullptr");
}

TEST_F(ApiExperimentalUnitTest, rgl_mesh_set_vertices)
{
    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_set_vertices(nullptr, vertices.data(), vertices.size()), "mesh != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_mesh_set_vertices(mesh, vertices.data(), vertices.size()), "mesh != nullptr");

    vertices.push_back({ .value = { 1.0, 2.0, 3.0 } });
    indices.push_back({ .value = { 1, 2, 3 } });

    EXPECT_RGL_SUCCESS(rgl_mesh_create(&mesh, vertices.data(), vertices.size(), indices.data(), indices.size()));

    vertices[0] = { .value = { 4.0, 5.0, 6.0 } };
    EXPECT_RGL_SUCCESS(rgl_mesh_set_vertices(mesh, vertices.data(), vertices.size()));
}

TEST_F(ApiExperimentalUnitTest, rgl_entity_create)
{
    EXPECT_RGL_INVALID_ARGUMENT(rgl_entity_create(nullptr, nullptr, mesh), "entity != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_entity_create(&entity, nullptr, mesh), "mesh != nullptr");

    vertices.push_back({ .value = { 1.0, 2.0, 3.0 } });
    indices.push_back({ .value = { 1, 2, 3 } });

    EXPECT_RGL_SUCCESS(rgl_mesh_create(&mesh, vertices.data(), vertices.size(), indices.data(), indices.size()));
    EXPECT_RGL_SUCCESS(rgl_entity_create(&entity, nullptr, mesh));

    EXPECT_RGL_SUCCESS(rgl_entity_destroy(entity));
    EXPECT_RGL_INVALID_ARGUMENT(rgl_entity_destroy(nullptr), "entity != nullptr");
    // TODO(piotr.rybicki): Fix return error 500 RGL_INTERNAL_EXCEPTION
    // EXPECT_RGL_INVALID_ARGUMENT(rgl_entity_destroy(entity), "entity != nullptr");
}

TEST_F(ApiExperimentalUnitTest, rgl_entity_set_pose)
{
    EXPECT_RGL_INVALID_ARGUMENT(rgl_entity_set_pose(entity, nullptr), "entity != nullptr");

    vertices.push_back({ .value = { 1.0, 2.0, 3.0 } });
    indices.push_back({ .value = { 1, 2, 3 } });

    EXPECT_RGL_SUCCESS(rgl_mesh_create(&mesh, vertices.data(), vertices.size(), indices.data(), indices.size()));
    EXPECT_RGL_SUCCESS(rgl_entity_create(&entity, nullptr, mesh));

    EXPECT_RGL_INVALID_ARGUMENT(rgl_entity_set_pose(entity, nullptr), "local_to_world_tf != nullptr");
    EXPECT_RGL_SUCCESS(rgl_entity_set_pose(entity, &identity));

    EXPECT_RGL_SUCCESS(rgl_entity_destroy(entity));
}

TEST_F(ApiExperimentalUnitTest, rgl_lidar_create)
{
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_destroy(lidar), "lidar != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_create(nullptr, nullptr, 0), "out_lidar != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_create(&lidar, nullptr, 0), "ray_transforms != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_create(&lidar, &identity, 0), "ray_transforms_count > 0");
    EXPECT_RGL_SUCCESS(rgl_lidar_create(&lidar, &identity, 1));

    EXPECT_RGL_SUCCESS(rgl_lidar_destroy(lidar));
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_destroy(nullptr), "lidar != nullptr");
    // TODO(piotr.rybicki): Fix return error 500 RGL_INTERNAL_EXCEPTION
    // EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_destroy(lidar), "lidar != nullptr");
}

TEST_F(ApiExperimentalUnitTest, rgl_lidar_set_range)
{
    EXPECT_RGL_SUCCESS(rgl_lidar_create(&lidar, &identity, 1));

    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_set_range(nullptr, 0.0), "lidar != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_set_range(lidar, std::nanf("0.0")), "!std::isnan(range)");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_set_range(lidar, -1.0), "range > 0");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_set_range(lidar, 0.0), "range > 0");
    EXPECT_RGL_SUCCESS(rgl_lidar_set_range(lidar, 1.0));

    EXPECT_RGL_SUCCESS(rgl_lidar_destroy(lidar));
}

TEST_F(ApiExperimentalUnitTest, rgl_lidar_set_pose)
{
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_set_pose(lidar, nullptr), "lidar != nullptr");
    EXPECT_RGL_SUCCESS(rgl_lidar_create(&lidar, &identity, 1));

    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_set_pose(lidar, nullptr), "local_to_world_tf != nullptr");
    EXPECT_RGL_SUCCESS(rgl_lidar_set_pose(lidar, &identity));

    EXPECT_RGL_SUCCESS(rgl_lidar_destroy(lidar));
}

TEST_F(ApiExperimentalUnitTest, rgl_lidar_output)
{
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_raytrace_async(nullptr, lidar), "lidar != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_get_output_size(lidar, nullptr), "lidar != nullptr");
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_get_output_data(lidar, rgl_format_t::RGL_FORMAT_XYZ, nullptr), "lidar != nullptr");
    EXPECT_RGL_SUCCESS(rgl_lidar_create(&lidar, &identity, 1));
    EXPECT_RGL_SUCCESS(rgl_lidar_raytrace_async(nullptr, lidar));
    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_get_output_size(lidar, nullptr), "out_size != nullptr");

    int hitpointCount = 1;
    EXPECT_RGL_SUCCESS(rgl_lidar_get_output_size(lidar, &hitpointCount));
    EXPECT_EQ(hitpointCount, 0);

    EXPECT_RGL_INVALID_ARGUMENT(rgl_lidar_get_output_data(lidar, rgl_format_t::RGL_FORMAT_XYZ, nullptr), "out_data != nullptr");

    rgl_vec3f results[1] = { 0 };
    EXPECT_RGL_SUCCESS(rgl_lidar_get_output_data(lidar, rgl_format_t::RGL_FORMAT_XYZ, results));
}