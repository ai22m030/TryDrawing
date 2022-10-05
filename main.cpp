#include <iostream>
#include <cstdio>
#include <vector>
#include <set>
#include <SDL.h>
#include <future>

using namespace std;

const float SCALE_X=4.0;
const float SCALE_Y=4.0;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

pair<int, int> mid;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

void drawLine(int x1, int y1, int x2, int y2) {
    //SDL_RenderClear(renderer);
    //SDL_RenderSetScale( renderer, 1.0, 1.0 );
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    //SDL_RenderPresent(renderer);
}

int quad(pair<int, int> p) {
    if (p.first >= 0 && p.second >= 0)
        return 1;
    if (p.first <= 0 && p.second >= 0)
        return 2;
    if (p.first <= 0 && p.second <= 0)
        return 3;
    return 4;
}

int orientation(pair<int, int> a, pair<int, int> b,
                pair<int, int> c) {
    int res = (b.second - a.second) * (c.first - b.first) -
              (c.second - b.second) * (b.first - a.first);

    if (res == 0)
        return 0;
    if (res > 0)
        return 1;
    return -1;
}

bool compare(pair<int, int> p1, pair<int, int> q1) {
    pair<int, int> p = make_pair(p1.first - mid.first,
                                 p1.second - mid.second);
    pair<int, int> q = make_pair(q1.first - mid.first,
                                 q1.second - mid.second);

    int one = quad(p);
    int two = quad(q);

    if (one != two)
        return (one < two);
    return (p.second * q.first < q.second * p.first);
}

vector<pair<int, int>> merger(vector<pair<int, int> > a,
                              vector<pair<int, int> > b) {
    // n1 -> number of points in polygon a
    // n2 -> number of points in polygon b
    int n1 = a.size(), n2 = b.size();

    // ia -> rightmost point of a
    int ia = 0, ib = 0;
    for (int i = 1; i < n1; i++)
        if (a[i].first > a[ia].first)
            ia = i;

    // ib -> leftmost point of b
    for (int i = 1; i < n2; i++)
        if (b[i].first < b[ib].first)
            ib = i;

    // finding the upper tangent
    int inda = ia, indb = ib;
    bool done = false;
    while (!done) {
        done = true;
        while (orientation(b[indb], a[inda], a[(inda + 1) % n1]) >= 0) {
            inda = (inda + 1) % n1;
        }

        while (orientation(a[inda], b[indb], b[(n2 + indb - 1) % n2]) <= 0) {
            indb = (n2 + indb - 1) % n2;
            done = false;
        }
    }

    int uppera = inda, upperb = indb;
    inda = ia, indb = ib;
    done = false;
    while (!done)//finding the lower tangent
    {
        done = true;
        while (orientation(a[inda], b[indb], b[(indb + 1) % n2]) >= 0)
            indb = (indb + 1) % n2;

        while (orientation(b[indb], a[inda], a[(n1 + inda - 1) % n1]) <= 0) {
            inda = (n1 + inda - 1) % n1;
            done = false;
        }
    }

    int lowera = inda, lowerb = indb;
    vector<pair<int, int>> ret;

    //ret contains the convex hull after merging the two convex hulls
    //with the points sorted in anti-clockwise order
    int ind = uppera;
    ret.push_back(a[uppera]);
    while (ind != lowera) {
        ind = (ind + 1) % n1;
        ret.push_back(a[ind]);
    }

    ind = lowerb;
    ret.push_back(b[lowerb]);
    while (ind != upperb) {
        ind = (ind + 1) % n2;
        ret.push_back(b[ind]);
    }
    return ret;
}

vector<pair<int, int>> bruteHull(vector<pair<int, int>> a) {
    // Take any pair of points from the set and check
    // whether it is the edge of the convex hull or not.
    // if all the remaining points are on the same side
    // of the line then the line is the edge of convex
    // hull otherwise not
    set<pair<int, int> > s;

    for (int i = 0; i < a.size(); i++) {
        for (int j = i + 1; j < a.size(); j++) {
            int x1 = a[i].first, x2 = a[j].first;
            int y1 = a[i].second, y2 = a[j].second;

            int a1 = y1 - y2;
            int b1 = x2 - x1;
            int c1 = x1 * y2 - y1 * x2;
            int pos = 0, neg = 0;
            for (auto &k: a) {
                if (a1 * k.first + b1 * k.second + c1 <= 0)
                    neg++;
                if (a1 * k.first + b1 * k.second + c1 >= 0)
                    pos++;
            }
            if (pos == a.size() || neg == a.size()) {
                s.insert(a[i]);
                s.insert(a[j]);
            }
        }
    }

    vector<pair<int, int>> ret;
    for (auto e: s)
        ret.push_back(e);

    // Sorting the points in the anti-clockwise order
    mid = {0, 0};
    int n = ret.size();
    for (int i = 0; i < n; i++) {
        mid.first += ret[i].first;
        mid.second += ret[i].second;
        ret[i].first *= n;
        ret[i].second *= n;
    }
    sort(ret.begin(), ret.end(), compare);
    for (int i = 0; i < n; i++)
        ret[i] = make_pair(ret[i].first / n, ret[i].second / n);

    return ret;
}

vector<pair<int, int>> divide(vector<pair<int, int>> a) {
    // If the number of points is less than 6 then the
    // function uses the brute algorithm to find the
    // convex hull
    if (a.size() < 6)
        return bruteHull(a);

    // left contains the left half points
    // right contains the right half points
    vector<pair<int, int>> left, right;
    for (int i = 0; i < a.size() / 2; i++)
        left.push_back(a[i]);
    for (int i = a.size() / 2; i < a.size(); i++)
        right.push_back(a[i]);

    // convex hull for the left and right sets
    vector<pair<int, int>> left_hull = divide(left);
    vector<pair<int, int>> right_hull = divide(right);

    // merging the convex hulls
    return merger(left_hull, right_hull);
}

void drawLines() {
    SDL_RenderSetScale( renderer, 1.0, 1.0 );
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, 320, 200, 300, 240);
    SDL_RenderDrawLine(renderer, 300, 240, 340, 240);
    SDL_RenderDrawLine(renderer, 340, 240, 320, 200);
}

void drawPoints() {
    SDL_RenderSetScale( renderer, SCALE_X, SCALE_Y );
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoint(renderer, 100 / SCALE_X, 100 / SCALE_Y);
}

int main(int argc, char* argv[]) {
    int points_count = 50;
    vector<pair<int, int> > a;
    srand(time(nullptr));
    std::cout << 10 + (rand() % 90) << std::endl;
    auto* points = new SDL_Point[points_count];
    for(int i = 0; i < points_count; i++) {
        points[i] = {(10 + (rand() % 90)), (10 + (rand() % 90))};
        a.push_back(make_pair(points[i].x, points[i].y));
    }
    /*
    a.push_back(make_pair(20, 20));
    a.push_back(make_pair(70, 30));
    a.push_back(make_pair(50, 20));
    a.push_back(make_pair(70, 50));
    a.push_back(make_pair(50, 60));
    a.push_back(make_pair(110, 50));
    a.push_back(make_pair(150, 70));
    a.push_back(make_pair(120, 30));
    a.push_back(make_pair(150, 30));
    a.push_back(make_pair(160, 50));
    a.push_back(make_pair(120, 70));
     */
    sort(a.begin(), a.end());

    if (SDL_Init(SDL_INIT_VIDEO) == 0) {

        if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
                                        SDL_WINDOW_SHOWN,
                                        &window,
                                        &renderer) == 0) {
            SDL_bool done = SDL_FALSE;
            //screenSurface = SDL_GetWindowSurface(window);

            //Fill the surface white
            //SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

            //Update the surface
            SDL_UpdateWindowSurface(window);

            vector<pair<int, int> > ans;
            bool first_run = true;
            bool second_run = true;
            SDL_Event event;
            ans = divide(a);
            //auto handle = std::async(std::launch::async,divide, a);
            while (!done) {
                if (first_run) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(renderer);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderSetScale(renderer, 6.0f, 4.0f);
                    SDL_RenderDrawPoints(renderer, points, points_count);
                    //drawLine(320, 200, 300, 240);
                    //drawLine( 300, 240, 340, 240);
                    //SDL_RenderDrawLine(renderer, 340, 240, 320, 200);
                    //drawLine(340, 240, 320, 200);
                    bool first_run_hull = true;
                    pair<int, int> prev_point;
                    pair<int, int> first_point;
                    for(auto p_tmp : ans) {
                        if(first_run_hull) {
                            prev_point = p_tmp;
                            first_point = p_tmp;
                            first_run_hull = false;
                        } else {
                            drawLine(prev_point.first, prev_point.second, p_tmp.first, p_tmp.second);
                            prev_point = p_tmp;
                        }
                    }

                    drawLine(prev_point.first, prev_point.second, first_point.first, first_point.second);
                    SDL_RenderPresent(renderer);

                    first_run = false;
                } else if (second_run) {

                    //ans = handle.get();
                    /*
                    cout << "convex hull:\n";
                    for (auto e: ans)
                        cout << e.first << " "
                             << e.second << endl;
                    */
                    second_run = false;
                }

                //drawLines();
                //drawPoints();
                //vector<pair<int, int> >

                if(!SDL_PollEvent(&event)) continue;

                switch (event.type) {
                    case SDL_QUIT:
                        done = SDL_TRUE;
                        break;
                }
            }
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
    SDL_Quit();
    return 0;
}

