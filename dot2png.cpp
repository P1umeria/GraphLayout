#define _CRT_SECURE_NO_WARNINGS

#include "nodesoup.hpp"
#include <algorithm>
#include "cairo.h"
#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include "getopt.h"
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <unordered_map>
#include <vector>


using namespace nodesoup;

using std::string;
using std::vector;
using std::cout;
using std::cerr;

// 绘图形状
enum Shape { circle, square };
// 算法选择
enum Method { fr, kk, yfh, sgym, adaption };

int my_optind = -1;
const char* my_optarg = "";
char my_getopt(int nargc, char* const* nargv, const char* options) {
    if (my_optind == -1) {
        my_optind = nargc;
    }
    while (my_optind > 1) {
        --my_optind;
        if (strlen(nargv[my_optind]) == 2 && nargv[my_optind][0] == '-') {
            if (my_optind != nargc - 1) {
                my_optarg = nargv[my_optind + 1];
                return nargv[my_optind][1];
            }
        }
    }
    return -1;
}

// 从字符串中获取属性值
void divide_str(string& s, vector<string>& vec) {
    bool in_com = false;
    string buffer = "";
    for (int i = 0; i < s.size(); ++i) {
        if (in_com == false) {
            if (s[i] >= 'a' && s[i] <= 'z' || s[i] >= 'A' && s[i] <= 'Z' || s[i] >= '0' && s[i] <= '9') {
                buffer.push_back(s[i]);
            }
            else if (buffer != "") {
                vec.push_back(buffer);
                buffer = "";
            }
            if (s[i] == '\"') {
                in_com = true;
            }
        }
        else {
            if (s[i] == '\"') {
                vec.push_back(buffer);
                in_com = false;
                buffer = "";
            }
            else if (s[i] == '\\') {
                buffer.push_back(s[i + 1]);
                ++i;
            }
            else {
                buffer.push_back(s[i]);
            }
        }
    }
    if (buffer != "") {
        vec.push_back(buffer);
    }
}

// 读取解析dot文件
// TODO: 对边和节点的颜色进行设定；对边的名称进行标记
adj_list_t read_from_dot(string filename, std::vector<string>& name_li, std::unordered_map<string, string>& line_li, std::vector<int>& shape_li) {
    std::ifstream ifs(filename);
    // 打不开文件
    if (!ifs.good()) {
        cerr << "Could not open file \"" << filename << "\"\n";
        exit(EXIT_FAILURE);
    }
    cout << "Read from file: " << filename << std::endl;
    adj_list_t g;
    std::unordered_map<string, vertex_id_t> names;
    // 名字到编号的映射创建和查询
    auto name_to_vertex_id = [&g, &names, &name_li, &shape_li](string name) -> vertex_id_t {
        // 去除列尾的分号
        while (name[name.size() - 1] == ';') {
            name.erase(name.end() - 1, name.end());
        }
        // 创建名字到编号的映射表
        vertex_id_t v_id;
        auto it = names.find(name);
        if (it != names.end()) {
            return (*it).second;
        }
        v_id = g.size();
        names.insert({ name, v_id });
        g.resize(v_id + 1);
        name_li.push_back(name);
        shape_li.push_back(0);
        return v_id;
        };

    auto remove_space = [](string& name) {
        // 去除列尾的空格
        while (name[name.size() - 1] == ' ') {
            name.erase(name.end() - 1, name.end());
        }
        // 去除列头的空格
        int idx;
        for (idx = 0; idx < name.size(); ++idx) {
            if (name[idx] != ' ') {
                break;
            }
        }
        if (idx != 0) {
            name.erase(name.begin(), name.begin() + idx);
        }
        };

    string line;
    // 跳过第一行
    std::getline(ifs, line);

    while (std::getline(ifs, line)) {
        remove_space(line);
        // 右括号结束
        if (line[0] == '}') {
            break;
        }
        // 判断是边属性行还是点属性行
        int line_idx = -1;
        bool in_com = false;

        for (int i = 0; i < line.size(); ++i) {
            if (in_com && line[i] == '\\') {
                i += 1;
            }
            else if (line[i] == '"') {
                in_com = !in_com;
            }
            else if (line[i] == '-' && !in_com) {
                if (i != 0 && line[i - 1] != '-') {
                    if (i != line.size() - 1 && (line[i + 1] == '-' || line[i + 1] == '>')) {
                        if (i != line.size() - 2 && line[i + 2] != '-' && line[i + 2] != '>') {
                            line_idx = i;
                            break;
                        }
                    }
                }
            }
        }
        vector<string> vec;
        divide_str(line, vec);
        if (line_idx != -1) {             // 边属性
            // 如果是新的，加入表中
            vertex_id_t v_id = name_to_vertex_id(vec[0]);
            vertex_id_t adj_id = name_to_vertex_id(vec[1]);
            // 如果是新的边，则加入邻接表中
            if (find(g[v_id].begin(), g[v_id].end(), adj_id) == g[v_id].end()) {
                g[v_id].push_back(adj_id);
                g[adj_id].push_back(v_id);
            }
            if (vec.size() >= 4 && vec[2] == "label") {
                if (v_id < adj_id) {
                    line_li[std::to_string(v_id) + "_" + std::to_string(adj_id)] = vec[3];
                }
                else {
                    line_li[std::to_string(adj_id) + "_" + std::to_string(v_id)] = vec[3];
                }
            }
        }
        else {                          // 点属性
            vertex_id_t v_id = name_to_vertex_id(vec[0]);
            int idx = 1;
            while (idx < vec.size()) {
                if (idx + 1 >= vec.size()) {
                    cerr << "Not correct dot file.\n";
                    exit(EXIT_FAILURE);
                }
                if (vec[idx] == "label") {
                    name_li[v_id] = vec[idx + 1];
                }
                else if (vec[idx] == "shape") {
                    int sh = 0;
                    if (vec[idx + 1] == "box") {
                        sh = 1;
                    }
                    shape_li[v_id] = sh;
                }
                else {
                    cerr << "Not correct dot file.\n";
                    exit(EXIT_FAILURE);
                }
                idx += 2;
            }
        }
    }
    return g;
}

void write_to_png(adj_list_t& g, vector<Point2D>& positions, vector<double>& radiuses, unsigned int width, unsigned int height,
    string filename, std::vector<string>& name_li, std::unordered_map<string, string>& line_li, vector<int>& shape_li) {
    cout << "Write to file: " << filename << std::endl;
    // 居中化(全部转化为正数)
    for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
        // cout << positions[v_id].x << " " << positions[v_id].y << std::endl;
        positions[v_id].x += width / 2.0;
        positions[v_id].y += height / 2.0;
    }
    // 创建surface表面
    cairo_surface_t* surface = cairo_image_surface_create(_cairo_format::CAIRO_FORMAT_ARGB32, width, height);
    // 上下文记录结构
    cairo_t* cr = cairo_create(surface);
    // 设置画笔参数
    cairo_set_source_rgb(cr, 255.0, 255.0, 255.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);

    cairo_select_font_face(cr, "Adobe Heiti Std", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 16.0);

    for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
        // 画边
        Point2D v_pos = positions[v_id];
        for (auto adj_id : g[v_id]) {
            if (adj_id < v_id) {
                continue;
            }
            cairo_set_source_rgb(cr, 0, 0, 1);
            string line_name = "";
            auto iterator = line_li.find(std::to_string(v_id) + "_" + std::to_string(adj_id));
            if (iterator != line_li.end() && iterator->second != "") {
                line_name = iterator->second;
            }
            // printf("%d %d\n", v_id, adj_id);
            Point2D adj_pos = positions[adj_id];
            cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
            cairo_move_to(cr, v_pos.x, v_pos.y);
            cairo_line_to(cr, adj_pos.x, adj_pos.y);
            cairo_stroke(cr);
            if (line_name != "") {
                cairo_set_source_rgb(cr, 1, 0, 0);
                cairo_text_extents_t extents;
                cairo_text_extents(cr, line_name.c_str(), &extents);
                cairo_move_to(cr, (v_pos.x + adj_pos.x) / 2.0 - extents.width - 1.0, (v_pos.y + adj_pos.y) / 2.0 + extents.height / 2.0);
                cairo_show_text(cr, line_name.c_str());
                cairo_stroke(cr);
            }
        }
    }

    for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
        // 画点
        cairo_set_source_rgb(cr, 0, 0, 0);
        Point2D v_pos = positions[v_id];

        cairo_text_extents_t extents;
        cairo_text_extents(cr, name_li[v_id].c_str(), &extents);
        if (shape_li[v_id] == 0) {
            double cur_radiuse = extents.width / 2.0;
            double rate = (extents.height) / (extents.width);
            if (extents.height / 2.0 > cur_radiuse) {
                cur_radiuse = extents.height / 2.0;
                rate = 1.0;
            }
            if (rate < 0.6) {
                rate = 0.6;
            }

            cur_radiuse += 8.0;
            radiuses[v_id] = cur_radiuse;
            cairo_scale(cr, 1.0, rate);
            cairo_arc(cr, v_pos.x, v_pos.y / rate, cur_radiuse, 0.0, 2.0 * M_PI);
            cairo_fill(cr);
            cairo_stroke(cr);

            cairo_set_source_rgb(cr, 1, 1, 1);
            // cairo_scale(cr, 1.0, rate);
            cairo_arc(cr, v_pos.x, v_pos.y / rate, cur_radiuse - 5.0, 0.0, 2.0 * M_PI);
            cairo_fill(cr);
            cairo_scale(cr, 1.0, 1.0 / rate);
            cairo_stroke(cr);
        }
        else {
            cairo_rectangle(cr, v_pos.x - extents.width / 2.0 - 6.0, v_pos.y - extents.height / 2.0 - 6.0, extents.width + 12.0, extents.height + 12.0);
            cairo_fill(cr);
            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_rectangle(cr, v_pos.x - extents.width / 2.0 - 3.0, v_pos.y - extents.height / 2.0 - 3.0, extents.width + 6.0, extents.height + 6.0);
            cairo_fill(cr);
        }

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_move_to(cr, v_pos.x - extents.width / 2.0, v_pos.y + extents.height / 2.0);
        cairo_show_text(cr, name_li[v_id].c_str());
        cairo_stroke(cr);
    }

    // 保存为图片
    if (cairo_surface_write_to_png(surface, filename.c_str()) != CAIRO_STATUS_SUCCESS) {
        cerr << "Could not write to file \"" << filename << "\"\n";
        exit(EXIT_FAILURE);
    }

    cairo_restore(cr);
    cairo_show_page(cr);
}

// 深度优先搜索判断是否存在环
bool judge_cycle(adj_list_t g, vector<int>& vec, int cur, int last) {
    if (vec[cur] == 0) {
        vec[cur] = 1;
    }
    else {
        return true;
    }
    for (int i = 0; i < g[cur].size(); ++i) {
        if (g[cur][i] == cur || g[cur][i] == last) {
            continue;
        }
        bool b = judge_cycle(g, vec, g[cur][i], cur);
        if (b) {
            return true;
        }
    }
    return false;
}

// 方法选择
Method method_choose(adj_list_t g) {
    if (g.size() > 2000) {
        return Method::yfh;
    }
    bool has_circle = false;
    vector<int> is_found(g.size(), 0);
    int cur = -1;
    int last = -1;
    for (int i = 0; i < g.size(); ++i) {
        if (is_found[i] == 0) {
            has_circle = judge_cycle(g, is_found, i, -1);
        }
        if (has_circle) {
            break;
        }
    }
    if (!has_circle) {
        return Method::sgym;
    }
    int edge_num = 0;
    for (int i = 0; i < g.size(); ++i) {
        edge_num += g[i].size();
    }
    if (edge_num >= 0.5 * g.size() * g.size()) {
        return Method::kk;
    }
    return Method::fr;
}


// 主要函数
void dot_to_png(
    string dot_filename,
    string png_filename,
    Method method,
    unsigned int width,
    unsigned int height,
    double k,
    double energy_threshold,
    int iters_count,
    bool animated,
    double stepRatio,
    double relativeStrength,
    int quadTreeMaxLevel,
    double barnesHutTheta,
    double convergenceThreshold, 
    bool refresh) {
    std::vector<string> name_li;
    std::unordered_map<string, string> line_li;
    vector<int> shape_li;
    // 读取文件
    adj_list_t g = read_from_dot(dot_filename, name_li, line_li, shape_li);
    vector<Point2D> positions;
    // 获取点的半径
    vector<double> radiuses = size_radiuses(g);
    // 如果是adaption，需要决定算法
    if (method == adaption) {
        method = method_choose(g);
        printf("Using: ");
        if (method == kk) {
            cout << "Kamada Kawai";
        }
        else if (method == fr) {
            cout << "Fruchterman reingold";
        }
        else if (method == yfh) {
            cout << "Yifan Hu";
        }
        else if (method == sgym) {
            cout << "Sugiyama";
        }
        cout << std::endl;
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    cout << "Laying out graph...\n";
    // Fruchterman-Reingold 
    if (method == Method::fr) {
        iter_callback_t cback = nullptr;
        char* frame_filename = nullptr;
        string frame_filename_format;

        // 逐帧输出
        if (animated) {
            size_t index = png_filename.rfind(".png");
            if (index == string::npos) {
                frame_filename_format = png_filename + "_%03d";
            }
            else {
                frame_filename_format = png_filename.substr(0, index) + "_%03d.png";
            }
            frame_filename = new char[frame_filename_format.size()];
            // 定制特别回调函数用于保存图片
            cback = [&g, &radiuses, width, height, iters_count, frame_filename, &frame_filename_format, &name_li, &line_li, &shape_li](vector<Point2D> positions, int iter) {
                if (iter % 2 != 0 && iter != 0 && iter != iters_count - 1) {
                    return;
                }
                sprintf(frame_filename, frame_filename_format.c_str(), iter);
                write_to_png(g, positions, radiuses, width, height, frame_filename, name_li, line_li, shape_li);
                };
        }

        if (k == -1.0) {
            k = 15.0;
        }

        start = std::chrono::system_clock::now();
        positions = fruchterman_reingold(g, width, height, iters_count, k, cback);
        end = std::chrono::system_clock::now();

        if (animated) {
            delete[] frame_filename;
        }
    }
    // Kamada-Kawai
    else if (method == Method::kk) {
        if (k == -1.0) {
            k = 300.0;
        }

        start = std::chrono::system_clock::now();
        positions = kamada_kawai(g, width, height, k, energy_threshold);
        end = std::chrono::system_clock::now();
    }
    // yifanhu
    else if (method == Method::yfh) {
        start = std::chrono::system_clock::now();
        positions = yifanhu(g, width, height, iters_count, stepRatio, relativeStrength, quadTreeMaxLevel, barnesHutTheta, convergenceThreshold);
        end = std::chrono::system_clock::now();
    }
    // sugiyama
    else {
        start = std::chrono::system_clock::now();
        positions = sugiyama(g, width, height);
        end = std::chrono::system_clock::now();
    }
    for (int v_id = 0; v_id < positions.size(); v_id++) {
        //printf("%f %f\n", positions[v_id].x, positions[v_id].y);
    }
    // 自适应计算长宽
    if (!refresh) {
        double min_x = positions[0].x;
        double max_x = positions[0].x;
        double min_y = positions[0].y;
        double max_y = positions[0].y;
        double max_label_size = 0.0;
        double rate = 0.2;
        // 创建surface表面
        cairo_surface_t* surface = cairo_image_surface_create(_cairo_format::CAIRO_FORMAT_ARGB32, width, height);
        // 上下文记录结构
        cairo_t* cr = cairo_create(surface);
        for (int v_id = 0; v_id < positions.size(); ++v_id) {
            if (positions[v_id].x < min_x) {
                min_x = positions[v_id].x;
            }
            if (positions[v_id].y < min_y) {
                min_y = positions[v_id].y;
            }
            if (positions[v_id].x > max_x) {
                max_x = positions[v_id].x;
            }
            if (positions[v_id].y > max_y) {
                max_y = positions[v_id].y;
            }
            cairo_text_extents_t extents;
            cairo_text_extents(cr, name_li[v_id].c_str(), &extents);
            if (extents.width > max_label_size) {
                max_label_size = extents.width;
            }
            if (extents.height > max_label_size) {
                max_label_size = extents.height;
            }
        }

        for (int v_id = 0; v_id < positions.size(); ++v_id) {
            for (int o_id = v_id + 1; o_id < positions.size(); ++o_id) {
                double dis = sqrt(pow(positions[v_id].x - positions[o_id].x, 2.0) + pow(positions[v_id].y - positions[o_id].y, 2.0));
                if (max_label_size * 2.0 / dis > rate) {
                    rate = max_label_size * 2.0 / dis;
                    //printf("Rate refresh, node dis, %d %d, %f\n", v_id, o_id, dis);
                }
            }
            for (int j = 0; j < g[v_id].size(); ++j) {
                int o_id = g[v_id][j];
                if (v_id == o_id) {
                    continue;
                }
                double dis = sqrt(pow(positions[v_id].x - positions[o_id].x, 2.0) + pow(positions[v_id].y - positions[o_id].y, 2.0));
                if (max_label_size * 3.0 / dis > rate) {
                    rate = max_label_size * 3.0 / dis;
                    //printf("Rate refresh, edge dis, %d %d, %f\n", v_id, o_id, dis);
                }
            }
        }
        width = (max_x - min_x) * rate + 2 * max_label_size;
        height = (max_y - min_y) * rate + 2 * max_label_size;
        double rate_x = 1.0;
        double rate_y = 1.0;
        if (width < 500) {
            rate_x = 500.0 / width;
            width = 500;
        }
        if (height < 500) {
            rate_y = 500.0 / height;
            height = 500;
        }
        for (int v_id = 0; v_id < positions.size(); ++v_id) {
            positions[v_id].x *= rate * rate_x;
            positions[v_id].y *= rate * rate_y;
        }
        //printf("%f %f %f %f %f %f\n", max_x, min_x, max_y, min_y, rate, max_label_size);
        //printf("%d %d\n", width, height);
    }

    write_to_png(g, positions, radiuses, width, height, png_filename, name_li, line_li, shape_li);

    unsigned int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Layout computed in " << ms << "ms\n";
}

void usage(string exec_name) {
    cerr << "Usage: " << exec_name << " [options] <in.dot> <out.png>\n";
    cerr << "Options:\n";
    cerr << "  -m <method>\t\tLayout method to use between Fruchterman Reingold, Kamada Kawai, Yifan Hu and Sugiyama [fr|kk|yfh|sgym, default: adaption]\n";
    cerr << "  -w <width>\t\tCanvas width in pixels [default: adaption]\n";
    cerr << "  -h <height>\t\tCanvas height in pixels [default: adaption]\n";
    cerr << "  -k <strength>\t\tStrength factor [default: 10 for fr, 300 for kk]\n";
    cerr << "  -i <nb_iterations>\tNumber of iterations for fr [default: 100]\n";
    cerr << "  -e <epsilon>\t\tEnergy threshold for kk [default: 1e-2]\n";
    cerr << "  -a\t\t\tOutput all intermediary frames for fr [default: false]\n";
}

int main(int argc, char* argv[]) {
    Method method = adaption;
    int width = 1024;
    int height = 1024;
    double k = -1.0;
    double energy_threshold = 1e-2;
    int iters_count = 300;
    bool animated = false;

    double stepRatio = 0.95;
    double relativeStrength = 0.2;
    int quadTreeMaxLevel = 20;
    double barnesHutTheta = 1.2;
    double convergenceThreshold = 1.0E-4;

    bool refresh_w_or_h = false;

    // 指令解析
    char opt;
    while ((opt = my_getopt(argc, argv, "m:w:h:k:e:i:a:s:r:l:b:c")) != -1) {
        switch (opt) {
        case 'm':
            if (string(my_optarg) == "fr") {
                method = fr;
            }
            else if (string(my_optarg) == "kk") {
                method = kk;
            }
            else if (string(my_optarg) == "yfh") {
                method = yfh;
            }
            else if (string(my_optarg) == "sgym") {
                method = sgym;
            }
            else {
                cerr << "Invalid method: \"" << my_optarg << "\"\n";
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'w':
            width = atoi(my_optarg);
            if (width <= 0) {
                cerr << "Invalid width: \"" << width << "\"\n";
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            refresh_w_or_h = true;
            break;
        case 'h':
            height = atoi(my_optarg);
            if (height <= 0) {
                cerr << "Invalid height: \"" << height << "\"\n";
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            refresh_w_or_h = true;
            break;
        case 'k':
            k = atof(my_optarg);
            cout << "K value = " << k << std::endl;
            if (k <= 0.0) {
                cerr << "Invalid k value: \"" << k << "\"\n";
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'i':
            iters_count = atoi(my_optarg);
            cout << "Iterator times = " << iters_count << std::endl;
            if (iters_count <= 0) {
                cerr << "Invalid iterations: \"" << iters_count << "\"\n";
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'e':
            energy_threshold = atof(my_optarg);
            cout << "Energy threshold = " << energy_threshold << std::endl;
            if (energy_threshold <= 0) {
                cerr << "Invalid energy threshold: \"" << energy_threshold << "\"\n";
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'a':
            animated = true;
            cout << "Animated = " << animated << std::endl;
            break;
        case 's':
            stepRatio = atof(my_optarg);
            cout << "Step ratio = " << stepRatio << std::endl;
            break;
        case 'r':
            relativeStrength = atof(my_optarg);
            cout << "Relative strength = " << relativeStrength << std::endl;
            break;
        case 'l':
            quadTreeMaxLevel = atoi(my_optarg);
            cout << "Quad tree max level = " << quadTreeMaxLevel << std::endl;
            break;
        case 'b':
            barnesHutTheta = atof(my_optarg);
            cout << "Barnes hut Theta = " << barnesHutTheta << std::endl;
            break;
        case 'c':
            convergenceThreshold = atof(my_optarg);
            cout << "Convergence threshold = " << convergenceThreshold << std::endl;
            break;
        default:
            cout << "ERROR\n";
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    cout << "Method: ";
    if (method == kk) {
        cout << "Kamada Kawai";
    }
    else if (method == fr) {
        cout << "Fruchterman reingold";
    }
    else if (method == yfh) {
        cout << "Yifan Hu";
    }
    else if (method == sgym) {
        cout << "Sugiyama";
    }
    else {
        cout << "Adaption";
    }
    if (refresh_w_or_h) {
        cout << std::endl << "Width: " << width << "  Height: " << height << std::endl;
    }
    else {
        cout << std::endl << "Adaption width and height" << std::endl;
    }
    

    if (argc - my_optind < 2) {
        cerr << "Missing positional arguments\n";
        cerr << "Optind = " << my_optind << ", ";
        cerr << "argc = " << argc << "\n";
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    char* dot_filename = argv[my_optind];
    char* png_filename = argv[my_optind + 1];
    dot_to_png(dot_filename, png_filename, method, width, height, k, energy_threshold,
        iters_count, animated, stepRatio, relativeStrength, quadTreeMaxLevel,
        barnesHutTheta, convergenceThreshold, refresh_w_or_h);
}
