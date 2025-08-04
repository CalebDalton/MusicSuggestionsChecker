#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <filesystem>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace std;

struct Song {
    std::string artist;
    std::string title;
    int energy;
    int danceability;
    int acousticness;
};

string normalize(const string &s) {
    string out;
    for (char c: s) {
        if (isalnum(static_cast<unsigned char>(c)) || c == ' ') {
            out += tolower(static_cast<unsigned char>(c));
        }
    }
    size_t firstAlpha = out.find_first_of("abcdefghijklmnopqrstuvwxyz");
    if (firstAlpha != string::npos) {
        out = out.substr(firstAlpha);
    } else {
        out = "zzz" + out;
    }
    return out;
}

std::vector<Song> loadSongs(const std::string& filename) {
    std::vector<Song> songs;
    std::filesystem::path current = std::filesystem::current_path();
    while (!std::filesystem::exists(current / "resources") && current.has_parent_path()) {
        current = current.parent_path();
    }
    std::filesystem::path csvPath = current / "resources" / filename;
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file\n";
        return songs;
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 100);

    std::string line;
    while (std::getline(file, line)) {
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            std::string artist = line.substr(0, commaPos);
            std::string title = line.substr(commaPos + 1);

            // Trim whitespace and remove quotes
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t\r\n\""));
                s.erase(s.find_last_not_of(" \t\r\n\"") + 1);
            };

            trim(artist);
            trim(title);

            if (!artist.empty() && !title.empty()) {
                Song s;
                s.artist = artist;
                s.title = title;
                s.energy = dist(rng);
                s.danceability = dist(rng);
                s.acousticness = dist(rng);
                songs.push_back(s);
            }
        }
    }

    std::cout << "CSV loaded from: \"" << csvPath.string() << "\"\n";
    return songs;
}

std::vector<Song> recommendSongs(const std::vector<Song>& songs, const Song& seed, int margin, bool useEnergy, bool useDance, bool useAcoustic, bool prioritizeSearch, const std::string& term) {
    std::vector<Song> recommendations;
    for (const auto& s : songs) {
        if (prioritizeSearch && term.size() > 0) {
            if (s.title.find(term) == std::string::npos && s.artist.find(term) == std::string::npos) continue;
        }

        bool match = true;
        if (useEnergy && (s.energy < seed.energy - margin || s.energy > seed.energy + margin)) match = false;
        if (useDance && (s.danceability < seed.danceability - margin || s.danceability > seed.danceability + margin)) match = false;
        if (useAcoustic && (s.acousticness < seed.acousticness - margin || s.acousticness > seed.acousticness + margin)) match = false;
        if (match) recommendations.push_back(s);
    }
    return recommendations;
}

double similarityScore(const Song& a, const Song& seed, bool useEnergy, bool useDance, bool useAcoustic) {
    double score = 0;
    if (useEnergy) score += std::abs(a.energy - seed.energy);
    if (useDance) score += std::abs(a.danceability - seed.danceability);
    if (useAcoustic) score += std::abs(a.acousticness - seed.acousticness);
    return score;
}

int partition(vector<Song> &songs, int low, int high, bool byTitle) {
    int randomIndex = low + rand() % (high - low + 1);
    swap(songs[randomIndex], songs[high]);
    string pivot = byTitle ? normalize(songs[high].title) : normalize(songs[high].artist);
    int i = low - 1;
    for (int j = low; j < high; j++) {
        string val = byTitle ? normalize(songs[j].title) : normalize(songs[j].artist);
        if (val <= pivot) {
            i++;
            swap(songs[i], songs[j]);
        }
    }
    swap(songs[i + 1], songs[high]);
    return i + 1;
}

void quickSort(vector<Song> &songs, int low, int high, bool byTitle) {
    while (low < high) {
        int pi = partition(songs, low, high, byTitle);
        if (pi - low < high - pi) {
            quickSort(songs, low, pi - 1, byTitle);
            low = pi + 1;
        } else {
            quickSort(songs, pi + 1, high, byTitle);
            high = pi - 1;
        }
    }
}

void merge(vector<Song> &songs, int l, int m, int r, bool byTitle) {
    int n1 = m - l + 1, n2 = r - m;
    vector<Song> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = songs[l + i];
    for (int j = 0; j < n2; j++) R[j] = songs[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        string leftVal = byTitle ? normalize(L[i].title) : normalize(L[i].artist);
        string rightVal = byTitle ? normalize(R[j].title) : normalize(R[j].artist);
        if (leftVal <= rightVal) songs[k++] = L[i++];
        else songs[k++] = R[j++];
    }
    while (i < n1) songs[k++] = L[i++];
    while (j < n2) songs[k++] = R[j++];
}

void mergeSort(vector<Song> &songs, int l, int r, bool byTitle) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(songs, l, m, byTitle);
        mergeSort(songs, m + 1, r, byTitle);
        merge(songs, l, m, r, byTitle);
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    std::vector<Song> songs = loadSongs("songdata.csv");

    // Setup ImGui + GLFW
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(1000, 800, "Music Suggestions", NULL, NULL);
    glfwMakeContextCurrent(window);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::GetStyle().ScaleAllSizes(2.5f);
    io.FontGlobalScale = 2.5f;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // GUI state
    static char searchBuf[128] = "";
    static int searchMode = 0; // 0 = Title, 1 = Artist
    static bool useEnergy = false, useDance = false, useAcoustic = false, prioritize = false;
    static int margin = 10;
    static int sortChoice = 0; // 0 = Artist, 1 = Title, 2 = Most Similar
    static bool recommendClicked = false;
    static std::vector<Song> recommendations;
    static Song seed;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static int sortAlgorithm = 0; // 0 = Quick Sort, 1 = Merge Sort
        static double sortTimeMs = 0.0;

        ImGui::Begin("Music Recommendations");

        ImGui::InputText("Search", searchBuf, IM_ARRAYSIZE(searchBuf));
        ImGui::RadioButton("Search by Title", &searchMode, 0); ImGui::SameLine();
        ImGui::RadioButton("Search by Artist", &searchMode, 1);

        ImGui::Checkbox("Use Energy", &useEnergy);
        ImGui::Checkbox("Use Danceability", &useDance);
        ImGui::Checkbox("Use Acousticness", &useAcoustic);
        ImGui::SliderInt("Margin of Error", &margin, 0, 50);
        ImGui::Checkbox("Prioritize Search Term", &prioritize);

        ImGui::Separator();
        ImGui::Text("Sort recommendations by:");
        ImGui::RadioButton("Artist", &sortChoice, 0); ImGui::SameLine();
        ImGui::RadioButton("Title", &sortChoice, 1); ImGui::SameLine();
        ImGui::RadioButton("Most Similar", &sortChoice, 2);

        ImGui::Text("Sort algorithm:");
        ImGui::RadioButton("Quick Sort", &sortAlgorithm, 0); ImGui::SameLine();
        ImGui::RadioButton("Merge Sort", &sortAlgorithm, 1);
        bool searchNotEmpty = strlen(searchBuf) > 0;

        if (!searchNotEmpty) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Get Recommendations")) {
            std::string search(searchBuf);
            std::vector<Song> matches;
            for (const auto& s : songs) {
                if (searchMode == 0) {
                    if (s.title.find(search) != std::string::npos) matches.push_back(s);
                } else {
                    if (s.artist.find(search) != std::string::npos) matches.push_back(s);
                }
            }
            if (!matches.empty()) {
                seed = matches[0];
            } else if (!songs.empty()) {
                seed = songs[rand() % songs.size()];
            }
            recommendations = recommendSongs(
                    songs, seed, margin,
                    useEnergy, useDance, useAcoustic,
                    prioritize, search
            );

            // Sorting and timing
            auto start = std::chrono::high_resolution_clock::now();
            if (sortAlgorithm == 0) { // Quick Sort
                if (sortChoice == 0)
                    quickSort(recommendations, 0, recommendations.size() - 1, false); // by artist
                else if (sortChoice == 1)
                    quickSort(recommendations, 0, recommendations.size() - 1, true);  // by title
                else
                    std::sort(recommendations.begin(), recommendations.end(), [&](const Song& a, const Song& b){
                        return similarityScore(a, seed, useEnergy, useDance, useAcoustic)
                               < similarityScore(b, seed, useEnergy, useDance, useAcoustic);
                    });
            } else { // Merge Sort
                if (sortChoice == 0)
                    mergeSort(recommendations, 0, recommendations.size() - 1, false); // by artist
                else if (sortChoice == 1)
                    mergeSort(recommendations, 0, recommendations.size() - 1, true);  // by title
                else
                    std::sort(recommendations.begin(), recommendations.end(), [&](const Song& a, const Song& b){
                        return similarityScore(a, seed, useEnergy, useDance, useAcoustic)
                               < similarityScore(b, seed, useEnergy, useDance, useAcoustic);
                    });
            }
            auto end = std::chrono::high_resolution_clock::now();
            sortTimeMs = std::chrono::duration<double, std::milli>(end - start).count();

            recommendClicked = true;
        }
        if (!searchNotEmpty) {
            ImGui::EndDisabled();
        }
        if (recommendClicked) {
            ImGui::Separator();
            ImGui::Text("Sort Time: %.3f ms", sortTimeMs);
            ImGui::Text("Total Recommendations: %d", (int)recommendations.size()); // <-- Add this line
            ImGui::Separator();
            ImGui::Text("Seed Song: %s - %s [E:%d D:%d A:%d]", seed.artist.c_str(), seed.title.c_str(), seed.energy, seed.danceability, seed.acousticness);
            ImGui::Text("Top 10 Recommendations:");
            int show = std::min(10, (int)recommendations.size());
            for (int i = 0; i < show; i++) {
                ImGui::BulletText("%s - %s [E:%d D:%d A:%d]",
                                  recommendations[i].artist.c_str(),
                                  recommendations[i].title.c_str(),
                                  recommendations[i].energy,
                                  recommendations[i].danceability,
                                  recommendations[i].acousticness
                );
            }
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}