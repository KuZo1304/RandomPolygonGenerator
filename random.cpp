//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cmath>
#include <time.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "delaunator.hpp"

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 960;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Texture* loadTexture( std::string path );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }

        //Create window
        gWindow = SDL_CreateWindow( "Random Polygons", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
            }
        }
    }

    return success;
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Nothing to load
    return success;
}

void close()
{
    //Destroy window    
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

SDL_Texture* loadTexture( std::string path )
{
    //The final texture
    SDL_Texture* newTexture = NULL;

    return newTexture;
}

bool sortPointsx(SDL_Point p1, SDL_Point p2)
{
    return (p1.x < p2.x);
}

bool sortPointsy(SDL_Point p1, SDL_Point p2)
{
    return (p1.y < p2.y);
}

bool sortPointsAngle(SDL_Point p1, SDL_Point p2)
{
    return ( atan2(p1.y, p1.x) < atan2(p2.y, p2.x) );
}

bool orientation(SDL_Point a, SDL_Point b, SDL_Point c)
{
     return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}

class tri_pair
{
public:
    size_t first;
    int second;
    tri_pair(size_t a, int b)
    {
        first = a;
        second = b;
    }
};

bool comp(tri_pair t1, tri_pair t2)
{
    return (t1.second<t2.second);
}


int main( int argc, char* args[] )
{
     int seed = time(NULL);
     srand(seed);
    // std::cout << seed << std::endl;
    // srand(1584983581);

    int numPoints = 100;
    if(argc==2)
    {
        numPoints=atoi(args[1]);
    }

    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {   
            //Main loop flag
            bool quit = false;

            //Event handler
            SDL_Event e;

            // Set up random points persistent accross entire loop
            std::vector<SDL_Point> points;
            float ptScale = 2.0f;
            for( int i = 0; i < numPoints; ++i )
            {
                SDL_Point temp;
                temp.x = rand() % (SCREEN_WIDTH - 100) + 50;
                temp.y = rand() % (SCREEN_HEIGHT - 100) + 50;
                temp.x /= ptScale;
                temp.y /= ptScale;
                points.push_back(temp);
            }
            //std::cout << orientation(points[1], points[2], points[3]) << std::endl;

            //Flag to determine which type of polygon to create
            enum typeCode{None, Monotone, Star, Random};
            typeCode polytype = None;

            //Data Structures to store polygons generated
            std::vector<SDL_Point> monoPoly;
            SDL_Point start;
            SDL_Point end;

            std::vector<SDL_Point> starPoly;
            SDL_Point centroid;

            std::vector<double> coords;
            for(int i=0; i<points.size(); i++)
            {
                coords.push_back(points[i].x);
                coords.push_back(points[i].y);
            }
            delaunator::Delaunator d(coords);

            std::vector<size_t> hull_vert;

            int cull = numPoints-1;
            int curr = 0;

            //While application is running
            while( !quit )
            {
                //Set Scale
                SDL_RenderSetScale( gRenderer, ptScale, ptScale);

                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
                SDL_RenderClear( gRenderer );

                //Draw the random points
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0x00, 0xFF );//yellow
                for(int i=0; i<numPoints; i++)
                {
                    SDL_RenderDrawPoint( gRenderer, points[i].x, points[i].y);
                }

                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                    }

                    if( e.type == SDL_KEYDOWN )
                    {
                        switch( e.key.keysym.sym )
                        {
                            // Clear Screen
                            case SDLK_c:
                            {
                                polytype = None;

                                monoPoly.clear();
                                starPoly.clear();
                                hull_vert.clear();
                                
                            }
                            break;

                            // Monotone Polygon
                            case SDLK_m:
                            {
                                polytype = Monotone;


                                monoPoly.clear();
                                std::vector<SDL_Point> sorted(points);
                                std::sort(sorted.begin(), sorted.end(), sortPointsx);

                                start = sorted.front();
                                end = sorted.back();
                                //std::cout << "Start: " << start.x << ", " << start.y << std::endl;
                                //std::cout << "End  : " << end.x << ", " << end.y << std::endl;

                                std::vector<SDL_Point> upper;
                                std::vector<SDL_Point> lower;

                                for(int i=1; i<numPoints-1; i++){
                                    if( orientation(start, end, sorted[i]) ) 
                                    {
                                        lower.push_back(sorted[i]);
                                    }
                                    else 
                                    {
                                        upper.push_back(sorted[i]);
                                    }
                                }

                                monoPoly.push_back(start);
                                if( lower.size() != 0)
                                {
                                    for(int i=0; i<lower.size(); i++)
                                    {
                                        monoPoly.push_back(lower[i]);
                                    }
                                }
                                monoPoly.push_back(end);
                                if( upper.size() != 0)
                                {
                                    for(int i=upper.size()-1; i>-1; --i)
                                    {
                                        monoPoly.push_back(upper[i]);
                                    }
                                }
                            }
                            break;

                            // Star Polygon
                            case SDLK_s:
                            polytype = Star;
                            {
                                starPoly.clear();

                                centroid.x = 0;
                                centroid.y = 0;

                                for( int i=0; i<numPoints; i++)
                                {
                                    centroid.x += points[i].x;
                                    centroid.y += points[i].y;
                                }
                                centroid.x /= numPoints;
                                centroid.y /= numPoints;
                               

                                std::vector<SDL_Point> shifted(points);
                                for( int i=0; i<numPoints; i++)
                                {
                                    shifted[i].x -= centroid.x;
                                    shifted[i].y -= centroid.y;
                                }

                                std::sort(shifted.begin(), shifted.end(), sortPointsAngle);

                                for( int i=0; i<shifted.size(); i++)
                                {
                                    SDL_Point temp = shifted[i];
                                    temp.x += centroid.x;
                                    temp.y += centroid.y;
                                    starPoly.push_back(temp);
                                }
                            }
                            break;

                            // Random Polygon
                            case SDLK_r:
                            {
                                hull_vert.clear();
                                curr = 0;
                                
                                delaunator::Delaunator d(coords);

                                polytype = Random;
                                size_t e = d.hull_start;
                                do {
                                    hull_vert.push_back(e);
                                    e = d.hull_next[e];
                                } while (e != d.hull_start);
                            }
                            break;

                            // Quit key
                            case SDLK_q:
                            quit = true;
                            break;
                        }
                        
                    }
                }


                //Call Subroutines
                //Monotone Polygon
                if (polytype == Monotone)
                {
                    SDL_SetRenderDrawColor( gRenderer, 0x00, 0x80, 0x00, 0xFF );//green
                    SDL_RenderDrawLine( gRenderer, start.x, start.y, end.x, end.y);

                    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );//white
                    for(int i=0; i<monoPoly.size()-1; i++)
                    {
                        SDL_RenderDrawLine( gRenderer, monoPoly[i].x, monoPoly[i].y, monoPoly[i+1].x, monoPoly[i+1].y);
                    }
                    SDL_RenderDrawLine( gRenderer, monoPoly.back().x, monoPoly.back().y, monoPoly.front().x, monoPoly.front().y);
                }

                if (polytype == Star)
                {
                    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );//red
                    SDL_RenderDrawPoint(gRenderer, centroid.x, centroid.y);
                    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);//white
                    for(int i=0; i<starPoly.size()-1; i++)
                    {
                        SDL_RenderDrawLine( gRenderer, starPoly[i].x, starPoly[i].y, starPoly[i+1].x, starPoly[i+1].y);
                    }
                    SDL_RenderDrawLine( gRenderer, starPoly[starPoly.size()-1].x, starPoly[starPoly.size()-1].y, starPoly[0].x, starPoly[0].y);

                }

                if (polytype == Random)
                {
                    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                    for(std::size_t i=0; i<d.triangles.size(); i+=3)
                    {
                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0x60 );//blue
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[i]],   d.coords[2 * d.triangles[i] + 1],
                                            d.coords[2 * d.triangles[i+1]], d.coords[2 * d.triangles[i+1] + 1]);

                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[i+1]],   d.coords[2 * d.triangles[i+1] + 1],
                                            d.coords[2 * d.triangles[i+2]], d.coords[2 * d.triangles[i+2] + 1]);

                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[i+2]],   d.coords[2 * d.triangles[i+2] + 1],
                                            d.coords[2 * d.triangles[i]], d.coords[2 * d.triangles[i] + 1]);
                    }

                    
                    for(int i=0; i<hull_vert.size(); i++)
                    {
                        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0x00, 0xFF );//yellow
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * hull_vert[i]],   d.coords[2 * hull_vert[i] + 1],
                                            d.coords[2 * hull_vert[(i+1) % hull_vert.size()]], d.coords[2 * hull_vert[(i+1) % hull_vert.size()] + 1]);
                    }
                    
 
                    std::vector<std::vector<size_t>> tri_store;
                    for(int i=0; i<d.triangles.size(); i+=3)
                    {
                        size_t tri1 = d.triangles[i];
                        size_t tri2 = d.triangles[i+1];
                        size_t tri3 = d.triangles[i+2];

                        int tri1_idx = -1;
                        int tri2_idx = -1;
                        int tri3_idx = -1;
                        int pos_count = 0;
                        for(int j=0; j<hull_vert.size(); j++)
                        {
                            if(hull_vert[j] == tri1)
                            {
                                tri1_idx = j;
                                pos_count++;
                            }
                            if(hull_vert[j] == tri2)
                            {
                                tri2_idx = j;
                                pos_count++;
                            }
                            if(hull_vert[j] == tri3)
                            {
                                tri3_idx = j;
                                pos_count++;
                            }
                        }


                        if(pos_count == 2)
                        {

                            size_t min_idx = -1;
                            size_t max_idx = -1;
                            size_t not_idx = -1;
                            if(tri1_idx == -1)
                            {
                                min_idx = std::min(tri2_idx, tri3_idx);
                                max_idx = std::max(tri2_idx, tri3_idx);
                                not_idx = tri1;
                            }
                            if(tri2_idx == -1)
                            {
                                min_idx = std::min(tri1_idx, tri3_idx);
                                max_idx = std::max(tri1_idx, tri3_idx);
                                not_idx = tri2;
                            }
                            if(tri3_idx == -1)
                            {
                                min_idx = std::min(tri1_idx, tri2_idx);
                                max_idx = std::max(tri1_idx, tri2_idx);
                                not_idx = tri3;
                            }

                            if( min_idx == 0 and max_idx == hull_vert.size()-1){
                                std::vector<size_t> temp = {i, max_idx, not_idx, 0};
                                tri_store.push_back(temp);
                            } else
                            if( (min_idx+1)%hull_vert.size() == max_idx )
                            {
                                std::vector<size_t> temp = {i, min_idx, not_idx, 0};
                                tri_store.push_back(temp);
                            }
                            
                        } else
                        if(pos_count == 3){
                            std::vector<size_t> sorter = {tri1_idx, tri2_idx, tri3_idx};
                            std::sort(sorter.begin(), sorter.end());
                            size_t min_idx = sorter.front();
                            size_t mid_idx = sorter[1];
                            size_t max_idx = sorter.back();

                            if(min_idx == 0 and max_idx == hull_vert.size()-1)
                            {
                                if(mid_idx == 1)
                                {
                                    std::vector<size_t> temp = {i, max_idx, 0, 1};
                                    tri_store.push_back(temp);
                                } else
                                {
                                    std::vector<size_t> temp = {i, mid_idx, 0, 1};
                                    tri_store.push_back(temp);
                                }
                            } else
                            {
                                std::vector<size_t> temp = {i, min_idx, 0, 1};
                                tri_store.push_back(temp);
                            }

                        }


                    }
                    //std::cout << tri_store.size() << std::endl;
                    
                    /*
                    std::vector<size_t> hull_tri;
                    std::vector<size_t> tri_idx;
                    for(int i=0; i<d.triangles.size(); i+=3)
                    {
                        size_t tri1 = d.triangles[i];
                        size_t tri2 = d.triangles[i+1];
                        size_t tri3 = d.triangles[i+2];

                        for(int j=0; j<hull_vert.size(); j++)
                        {
                            size_t ver1 = hull_vert[j];
                            size_t ver2 = hull_vert[(j+1) % hull_vert.size()];

                            if(tri1 == ver1 && tri2 == ver2)
                            {
                                tri_idx.push_back(i);
                                hull_tri.push_back(ver1);
                                hull_tri.push_back(ver2);
                                hull_tri.push_back(tri3);
                                break;
                            } else
                            if(tri2 == ver1 && tri3 == ver2)
                            {
                                tri_idx.push_back(i);
                                hull_tri.push_back(ver1);
                                hull_tri.push_back(ver2);
                                hull_tri.push_back(tri1);
                                break;
                            } else
                            if(tri3 == ver1 && tri1 == ver2)
                            {
                                tri_idx.push_back(i);
                                hull_tri.push_back(ver1);
                                hull_tri.push_back(ver2);
                                hull_tri.push_back(tri2);
                                break;
                            } else
                            if(tri2 == ver1 && tri1 == ver2)
                            {
                                tri_idx.push_back(i);
                                hull_tri.push_back(ver1);
                                hull_tri.push_back(ver2);
                                hull_tri.push_back(tri3);
                                break;
                            } else
                            if(tri3 == ver1 && tri2 == ver2)
                            {
                                tri_idx.push_back(i);
                                hull_tri.push_back(ver1);
                                hull_tri.push_back(ver2);
                                hull_tri.push_back(tri1);
                                break;
                            } else
                            if(tri1 == ver1 && tri3 == ver2)
                            {
                                tri_idx.push_back(i);
                                hull_tri.push_back(ver1);
                                hull_tri.push_back(ver2);
                                hull_tri.push_back(tri2);
                                break;
                            } 

                        }
                    }
                    */
                    
                    
                 
                    /*
                    for(std::size_t i=0; i<hull_tri.size(); i+=3)
                    {
                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0xFF );//blue
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * hull_tri[i]],   d.coords[2 * hull_tri[i] + 1],
                                            d.coords[2 * hull_tri[i+1]], d.coords[2 * hull_tri[i+1] + 1]);

                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x80, 0x00, 0xFF );//green
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * hull_tri[i+1]],   d.coords[2 * hull_tri[i+1] + 1],
                                            d.coords[2 * hull_tri[i+2]], d.coords[2 * hull_tri[i+2] + 1]);

                        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xC0, 0xCB, 0xFF );//pink
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * hull_tri[i+2]],   d.coords[2 * hull_tri[i+2] + 1],
                                            d.coords[2 * hull_tri[i]], d.coords[2 * hull_tri[i] + 1]);
                    }
                    */
                    
                    
                    /*
                    for(std::size_t i=0; i<tri_store.size(); i++)
                    {
                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0xFF );//blue
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[tri_store[i][0]]],   d.coords[2 * d.triangles[tri_store[i][0]] + 1],
                                            d.coords[2 * d.triangles[tri_store[i][0]+1]], d.coords[2 * d.triangles[tri_store[i][0]+1] + 1]);

                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x80, 0x00, 0xFF );//green
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[tri_store[i][0]+1]],   d.coords[2 * d.triangles[tri_store[i][0]+1] + 1],
                                            d.coords[2 * d.triangles[tri_store[i][0]+2]], d.coords[2 * d.triangles[tri_store[i][0]+2] + 1]);

                        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xC0, 0xCB, 0xFF );//pink
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[tri_store[i][0]+2]],   d.coords[2 * d.triangles[tri_store[i][0]+2] + 1],
                                            d.coords[2 * d.triangles[tri_store[i][0]]], d.coords[2 * d.triangles[tri_store[i][0]] + 1]);
                    }
                    */
                    /*
                    for(std::size_t i=0; i<tri_idx.size(); i++)
                    {
                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0xFF );//blue
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[tri_idx[i]]],   d.coords[2 * d.triangles[tri_idx[i]] + 1],
                                            d.coords[2 * d.triangles[tri_idx[i]+1]], d.coords[2 * d.triangles[tri_idx[i]+1] + 1]);

                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x80, 0x00, 0xFF );//green
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[tri_idx[i]+1]],   d.coords[2 * d.triangles[tri_idx[i]+1] + 1],
                                            d.coords[2 * d.triangles[tri_idx[i]+2]], d.coords[2 * d.triangles[tri_idx[i]+2] + 1]);

                        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xC0, 0xCB, 0xFF );//pink
                        SDL_RenderDrawLine( gRenderer,
                                            d.coords[2 * d.triangles[tri_idx[i]+2]],   d.coords[2 * d.triangles[tri_idx[i]+2] + 1],
                                            d.coords[2 * d.triangles[tri_idx[i]]], d.coords[2 * d.triangles[tri_idx[i]] + 1]);
                    }
                    */

                    
                    if(curr < cull)
                    {
                        int rem_idx = rand()%tri_store.size();
                        std::vector<size_t> rem_tri = {tri_store[rem_idx][0], tri_store[rem_idx][1], tri_store[rem_idx][2], tri_store[rem_idx][3]};

                        if(rem_tri[3] == 0)
                        {
                            hull_vert.insert(hull_vert.begin() + (rem_tri[1]+1), rem_tri[2]);
                            curr++;
                        } else
                        if(rem_tri[3] == 1)
                        {
                            //hull_vert.erase(hull_vert.begin()+(rem_tri[1]+1));
                        }

                    }
                    


                 
                    usleep(1000);
                }
                /*
                //Render red filled quad
                SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );//red
                SDL_RenderFillRect( gRenderer, &fillRect );

                //Render green outlined quad
                SDL_Rect outlineRect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3 };
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );        
                SDL_RenderDrawRect( gRenderer, &outlineRect );
                
                //Draw blue horizontal line
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0xFF );        
                SDL_RenderDrawLine( gRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2 );
                */


                SDL_RenderSetScale( gRenderer, 1.0f, 1.0f);
                //Update screen
                SDL_RenderPresent( gRenderer );
            }
        }
    }

    //Free resources and close SDL
    close();

    return 0;
}
