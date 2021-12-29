#pragma once

#include <vector>
#include <string>
#include <fstream>


class figure_model
{
    struct vertex_data
    {
        int vertex_index;
        int texture_index;
        int normal_index;
    };

    struct Vertex
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    };

    std::vector<std::string> split(std::string s, std::string delimiter) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector< std::string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

    std::vector<Vertex> vertices;
    std::vector<std::vector<float>> normals;
    std::vector<Vertex> texture;
    std::vector<std::vector<vertex_data>> faces;

public:

    int count() const
    {
        return faces.size() * 3;

    }
    figure_model(const std::string& path)
    {
        std::ifstream file(path);
        std::string str;
        while (std::getline(file, str))
        {
            auto arr = split(str, " ");
            if (arr.size() < 3) continue;;
            if (arr[0] == "v")
            {
                vertices.push_back({ std::stof(arr[1]), std::stof(arr[2]), std::stof(arr[3]) });
            }
            else if (arr[0] == "vt")
            {
                texture.push_back({ std::stof(arr[1]), std::stof(arr[2]),  0 });
            }
            else if (arr[0] == "vn")
            {
                normals.push_back({ std::stof(arr[1]), std::stof(arr[2]), std::stof(arr[3]) });
            }
            else if (arr[0] == "f")
            {
                auto v = std::vector<vertex_data>();
                for (int i = 1; i < 4; i++)
                {
                    auto s = split(arr[i], "/");
                    v.push_back({ std::stoi(s[0]) - 1, std::stoi(s[1]) - 1,std::stoi(s[2]) - 1 });
                }
                faces.push_back(v);

            }
        }
    }

    std::tuple<Vertex*, int> get_vertices_struct() const
    {
        auto fsize = faces[0].size();
        auto res = new Vertex[faces.size() * fsize];
        for (int i = 0; i < faces.size(); i++)
        {
            for (int j = 0; j < fsize; j++)
            {
                res[fsize * i + j] = vertices[faces[i][j].vertex_index];
            }
        }
        return { res, fsize * faces.size() * sizeof(Vertex) };
    }

    std::tuple<std::vector<std::vector<float>>, int> get_vertices() const
    {
        auto fsize = faces[0].size();
        // auto res = new Vertex[];
        auto res = std::vector<std::vector<float>>(faces.size() * fsize);
        for (int i = 0; i < faces.size(); i++)
        {
            for (int j = 0; j < fsize; j++)
            {
                auto x = vertices[faces[i][j].vertex_index].x;
                auto y = vertices[faces[i][j].vertex_index].y;
                auto z = vertices[faces[i][j].vertex_index].z;
                res[fsize * i + j] = { x,y,z };
            }
        }
        return { res, fsize * faces.size() * sizeof(Vertex) };
    }

    std::tuple<Vertex*, int>  get_texture() const
    {
        auto fsize = faces[0].size();
        auto res = new Vertex[faces.size() * fsize];
        for (int i = 0; i < faces.size(); i++)
        {
            for (int j = 0; j < fsize; j++)
            {
                res[fsize * i + j] = texture[faces[i][j].texture_index];
            }
        }
        return { res, fsize * faces.size() * sizeof(Vertex) };
    }

    std::tuple<std::vector<std::vector<float>>, int>  get_normals() const
    {
        auto res = std::vector<std::vector<float>>(faces.size() * 3);
        int j = 0;
        for (int i = 0; i < faces.size(); i++)
        {
            auto face = faces[i];
            auto v1 = faces[i][0];
            auto v2 = faces[i][1];
            auto v3 = faces[i][2];
            auto n1 = normals[v1.normal_index];
            auto n2 = normals[v2.normal_index];
            auto n3 = normals[v3.normal_index];
            res[j] = n1;
            res[j + 1] = n2;
            res[j + 2] = n3;
            j += 3;
        }
        return { res,  faces.size() * 3 * sizeof(float) };
    }

    std::vector<float> get_vert_and_normals()
    {
        auto verts = std::get<0>(get_vertices());
        auto normals = std::get<0>(get_normals());
        std::vector<float> res;
        for (int i = 0; i < normals.size(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                res.push_back(verts[i][j]);
            }
            for (int j = 0; j < 3; j++)
            {
                res.push_back(normals[i][j]);
            }
        }
        return res;
    }

    std::vector<float> get_all_values()
    {
        std::vector<float> res;
        for (int i = 0; i < faces.size(); ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                res.push_back(vertices[faces[i][j].vertex_index].x);
                res.push_back(vertices[faces[i][j].vertex_index].y);
                res.push_back(vertices[faces[i][j].vertex_index].z);
                res.push_back(texture[faces[i][j].texture_index].x);
                res.push_back(1 - texture[faces[i][j].texture_index].y);
                res.push_back(normals[faces[i][j].normal_index][0]);
                res.push_back(normals[faces[i][j].normal_index][1]);
                res.push_back(normals[faces[i][j].normal_index][2]);
            }
        }
        return res;
    }
};
