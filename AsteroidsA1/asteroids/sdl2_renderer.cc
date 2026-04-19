#include "sdl2_renderer.h"
#include <array>
#include <cmath>
#include <span>
#include <utility>

namespace {
// Simple helper used for all line/point colors in this renderer.
// Dadurch wird die Farbwahl im ganzen Code einheitlich und lesbar.
template <typename T, std::size_t N>
constexpr std::size_t countof(const T (&)[N]) noexcept { return N; }

inline void setColor(SDL_Renderer * renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 0xFF) {
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
}

void SDL2Renderer::renderSpaceship(Vector2df position, float angle) {
    static std::array<SDL_Point, 6> ship_points{SDL_Point{-6, 3},
                                              SDL_Point{-6,-3},
                                              SDL_Point{-10,-6},
                                              SDL_Point{ 14, 0},
                                              SDL_Point{-10, 6},
                                              SDL_Point{-6, 3}};
  
  std::array<SDL_Point, ship_points.size()> points;

  // Visuelle Skalierung nur für die Darstellung, nicht für die Spielphysik.
  // Die Hitbox der Objekte bleibt dadurch unverändert.
  float visual_scale = 1.4f; // Schiff optisch etwas größer zeichnen
  float cos_angle = std::cos(angle);
  float sin_angle = std::sin(angle);
  for (size_t i = 0; i < ship_points.size(); i++) {
    float x = ship_points[i].x * visual_scale;
    float y = ship_points[i].y * visual_scale;
    points[i].x = (cos_angle * x - sin_angle * y) + position[0];
    points[i].y = (sin_angle * x + cos_angle * y) + position[1];
  }
  // Hauptkörper des Schiffs in einer hellen Cyan-Grün-Farbe.
  setColor(renderer, 0x30, 0xFF, 0xB0);
  SDL_RenderDrawLines(renderer, points.data(), points.size());

  // Ein zusätzlicher Mittelsteg als Akzentlinie.
  std::array<SDL_Point, 2> spine{{SDL_Point{-6, 0}, SDL_Point{14, 0}}};
  std::array<SDL_Point, spine.size()> spine_points;
  for (size_t i = 0; i < spine.size(); i++) {
    float x = spine[i].x * visual_scale;
    float y = spine[i].y * visual_scale;
    spine_points[i].x = (cos_angle * x - sin_angle * y) + position[0];
    spine_points[i].y = (sin_angle * x + cos_angle * y) + position[1];
  }
  setColor(renderer, 0x00, 0xFF, 0xFF);
  SDL_RenderDrawLines(renderer, spine_points.data(), spine_points.size());

}

void SDL2Renderer::render(Spaceship * ship) {
  static SDL_Point flame_points[] { {-6, 3}, {-12, 0}, {-6, -3} };
  std::array<SDL_Point, countof(flame_points)> points;

  if (! ship->is_in_hyperspace()) {
    if (ship->is_accelerating()) {
      // Beschleunigungs-Feuer für das Schiff, damit das Schiff lebendiger wirkt.
      setColor(renderer, 0xFF, 0x90, 0x20);
      float cos_angle = std::cos(ship->get_angle());
      float sin_angle = std::sin(ship->get_angle());
      for (size_t i = 0; i < points.size(); i++) {
        float x = flame_points[i].x;
        float y = flame_points[i].y;
        points[i].x = (cos_angle * x - sin_angle * y) + ship->get_position()[0];
        points[i].y = (sin_angle * x + cos_angle * y) + ship->get_position()[1];
      }
      SDL_RenderDrawLines(renderer, points.data(), points.size());
    }
    renderSpaceship(ship->get_position(), ship->get_angle());
  }
}

void SDL2Renderer::render(Saucer * saucer) {
  static SDL_Point saucer_points[] = { {-16, -6}, {16, -6}, {40, 6}, {-40, 6}, {-16, 18}, {16, 18},
                                       {40, 6}, {16, -6}, {8, -18}, {-8, -18}, {-16, -6}, {-40, 6} };
  
  std::array<SDL_Point, countof(saucer_points)> points;

  Vector2df position = saucer->get_position();
  float scale = 0.5f;
  if ( saucer->get_size() == 0 ) {
    scale = 0.25f;
  }

  // Farbe wechselt je nach Größe, damit kleine und große UFOs besser unterscheidbar sind.
  Uint8 red = 0xFF;
  Uint8 green = 0x40;
  Uint8 blue = (saucer->get_size() == 0 ? 0xFF : 0x80);
  setColor(renderer, red, green, blue);

  for (size_t i = 0; i < points.size(); i++) {
    float x = saucer_points[i].x;
    float y = saucer_points[i].y;
    points[i].x = scale * x + position[0];
    points[i].y = scale * y + position[1];
  }
  SDL_RenderDrawLines(renderer, points.data(), points.size());

  std::array<SDL_Point, 3> dome{{SDL_Point{-8,0}, SDL_Point{0,-10}, SDL_Point{8,0}}};
  std::array<SDL_Point, dome.size()> dome_points;
  for (size_t i = 0; i < dome.size(); i++) {
    dome_points[i].x = dome[i].x * scale + position[0];
    dome_points[i].y = dome[i].y * scale + position[1];
  }
  setColor(renderer, 0xFF, 0xFF, 0xFF);
  SDL_RenderDrawLines(renderer, dome_points.data(), dome_points.size());

  if (((static_cast<int>(position[0]) / 10) % 2) == 0) {
    SDL_RenderDrawLine(renderer, position[0] - 10, position[1], position[0] + 10, position[1]);
  } else {
    SDL_RenderDrawLine(renderer, position[0], position[1] - 10, position[0], position[1] + 10);
  }
}


void SDL2Renderer::render(Torpedo * torpedo) {
  // Die Torpedos bleiben als kleiner roter Punkt sichtbar.
  setColor(renderer, 0xFF, 0x20, 0x20);
  SDL_RenderDrawPoint(renderer, torpedo->get_position()[0], torpedo->get_position()[1]);
  SDL_RenderDrawPoint(renderer, torpedo->get_position()[0] + 1, torpedo->get_position()[1]);
  SDL_RenderDrawPoint(renderer, torpedo->get_position()[0], torpedo->get_position()[1] - 1);
  SDL_RenderDrawPoint(renderer, torpedo->get_position()[0], torpedo->get_position()[1] + 1);
  SDL_RenderDrawPoint(renderer, torpedo->get_position()[0] - 1, torpedo->get_position()[1]);
}
  
void SDL2Renderer::render(Asteroid * asteroid) {
  static SDL_Point asteroids_points1[] = {
    { 0, -12}, {16, -24}, {32, -12}, {24, 0}, {32, 12}, {8, 24}, {-16, 24}, {-32, 12}, {-32, -12}, {-16, -24}, {0, -12}
  };   
  static SDL_Point asteroids_points2[] = {
    { 16, -6}, {32, -12}, {16, -24}, {0, -16}, {-16, -24}, {-24, -12}, {-16, -0}, {-32, 12}, {-16, 24}, {-8, 16}, {16, 24}, {32, 6}, {16, -6}
  }; 
  static SDL_Point asteroids_points3[] = {
    {-16, 0}, {-32, 6}, {-16, 24}, {0, 6}, {0, 24}, {16, 24}, {32, 6}, {32, 6}, {16, -24}, {-8, -24}, {-32, -6}, {-16, 0}
  };
  static SDL_Point asteroids_points4[] = {  
    {8,0}, {32,-6}, {32, -12}, {8, -24}, {-16, -24}, {-8, -12}, {-32, -12}, {-32, 12}, {-16, 24}, {8, 16}, {16, 24}, {32, 12}, {8, 0}
  };
  static size_t sizes[] = {countof(asteroids_points1),
                           countof(asteroids_points2),
                           countof(asteroids_points3),
                           countof(asteroids_points4) };
  size_t size = sizes[ asteroid->get_rock_type() ];
  SDL_Point * asteroids_points = asteroids_points1;
  if ( asteroid->get_rock_type() == 1 ) asteroids_points = asteroids_points2;
  if ( asteroid->get_rock_type() == 2 ) asteroids_points = asteroids_points3;
  if ( asteroid->get_rock_type() == 3 ) asteroids_points = asteroids_points4;
 
  SDL_Point points[countof(asteroids_points4)];
  
  float scale = (asteroid->get_size() == 3 ? 1.0f : ( asteroid->get_size() == 2 ? 0.5f : 0.25f ));
  Vector2df position = asteroid->get_position();

  // Farbverlauf für Asteroiden abhängig von ihrer Größe:
  // groß = violett-blau, mittel = orange, klein = helles orange/rot.
  Uint8 r = 0x66;
  Uint8 g = 0xCC;
  Uint8 b = 0xFF;
  if (asteroid->get_size() == 3) {
    r = 0x88;
    g = 0x44;
    b = 0xFF;
  } else if (asteroid->get_size() == 2) {
    r = 0xFF;
    g = 0x88;
    b = 0x22;
  } else {
    r = 0xFF;
    g = 0x44;
    b = 0x00;
  }
  setColor(renderer, r, g, b);
  for (size_t i = 0; i < size; i++) {
    points[i].x = scale * asteroids_points[i].x + position[0];
    points[i].y = scale * asteroids_points[i].y + position[1];
  }
  SDL_RenderDrawLines(renderer, points, size);

  if (size >= 5) {
    // Zusätzliche Risslinie für große Asteroiden als visuelles Detail.
    SDL_Point crack[] = {{-8, -8}, {0, 0}, {8, -4}};
    std::array<SDL_Point, countof(crack)> crack_points;
    for (size_t i = 0; i < crack_points.size(); i++) {
      crack_points[i].x = scale * crack[i].x + position[0];
      crack_points[i].y = scale * crack[i].y + position[1];
    }
    setColor(renderer, r / 2, g / 2, b / 2);
    SDL_RenderDrawLines(renderer, crack_points.data(), crack_points.size());
  }
}


void SDL2Renderer::render(SpaceshipDebris * debris) {
  static SDL_Point ship_points[6][2] = {{ SDL_Point{-2, -1}, SDL_Point{-10, 7} }, 
                                        { SDL_Point{3, 1}, SDL_Point{7, 8} },
                                        { SDL_Point{0, 3}, SDL_Point{6, 1} },
                                        { SDL_Point{3, -1}, SDL_Point{ -5, -7} },
                                        { SDL_Point{0, -4}, SDL_Point{-6, -6} },
                                        { SDL_Point{-2, 2}, SDL_Point{2, 5} } };
  static std::array<Vector2df, 6> debris_direction = { Vector2df{-40, -23}, Vector2df{50, 15}, Vector2df{0, 45},
                                                       Vector2df{60, -15}, Vector2df{10, -52}, Vector2df{-40, 30} };
  Vector2df position = debris->get_position();
  std::array<SDL_Point, 4> points;
  float progress = debris->get_time_to_delete() / SpaceshipDebris::TIME_TO_DELETE;
  Uint8 brightness = static_cast<Uint8>(255.0f * (1.0f - progress));
  // Debris wird beim Zerfall nach und nach dunkler.
  setColor(renderer, brightness, brightness, 0x40);
  float scale =  0.2f * (SpaceshipDebris::TIME_TO_DELETE - debris->get_time_to_delete());
  for (size_t i = 0; i < debris_direction.size(); i++) {
    points[0].x = scale * debris_direction[i][0] + ship_points[i][0].x + position[0];
    points[0].y = scale * debris_direction[i][1] + ship_points[i][0].y + position[1];
    points[1].x = scale * debris_direction[i][0] + ship_points[i][1].x + position[0];
    points[1].y = scale * debris_direction[i][1] + ship_points[i][1].y + position[1];
    if ( debris->get_time_to_delete() >= 0.5f * i )  {
      SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[1].x, points[1].y );    
    }
  }
                                  
}

void SDL2Renderer::render(Debris * debris) {
  static SDL_Point debris_points[] = { {-32, 32}, {-32, -16}, {-16, 0}, {-16, -32}, {-8, 24}, {8, -24}, {24, 32}, {24, -24}, {24, -32}, {32, -8} };

  static SDL_Point point;
  Vector2df position = debris->get_position();
  float factor = 1.0f - debris->get_time_to_delete() / Debris::TIME_TO_DELETE;
  Uint8 brightness = static_cast<Uint8>(120 + 135.0f * factor);
  // Verblassende Partikel, damit das Trümmerfeld dynamisch wirkt.
  setColor(renderer, brightness, brightness, 0xFF);
  for (size_t i = 0; i < countof(debris_points); i++) {
    point.x = (Debris::TIME_TO_DELETE - debris->get_time_to_delete()) * debris_points[i].x + position[0];
    point.y = (Debris::TIME_TO_DELETE - debris->get_time_to_delete()) * debris_points[i].y + position[1];
    SDL_RenderDrawPoint(renderer, point.x, point.y);
  }
}

void SDL2Renderer::renderFreeShips() {
  constexpr float FREE_SHIP_X = 128;
  constexpr float FREE_SHIP_Y = 64;
  Vector2df position = {FREE_SHIP_X, FREE_SHIP_Y};
  
  for (int i = 0; i < game.get_no_of_ships(); i++) {
    renderSpaceship( position, -PI / 2.0 );
    position[0] += 20.0;
  }
}




void SDL2Renderer::renderScore() {
  constexpr float SCORE_X = 128 - 48;
  constexpr float SCORE_Y = 48 - 4;
  
  static SDL_Point digit_0[] = { {0,-8}, {4,-8}, {4,0}, {0,0}, {0, -8} };
  static SDL_Point digit_1[] = { {4,0}, {4,-8} };
  static SDL_Point digit_2[] = { {0,-8}, {4,-8}, {4,-4}, {0,-4}, {0,0}, {4,0}  };
  static SDL_Point digit_3[] = { {0,0}, {4, 0}, {4,-4}, {0,-4}, {4,-4}, {4, -8}, {0, -8}  };
  static SDL_Point digit_4[] = { {4,0}, {4,-8}, {4,-4}, {0,-4}, {0,-8}  };
  static SDL_Point digit_5[] = { {0,0}, {4,0}, {4,-4}, {0,-4}, {0,-8}, {4, -8}  };
  static SDL_Point digit_6[] = { {0,-8}, {0,0}, {4,0}, {4,-4}, {0,-4} };
  static SDL_Point digit_7[] = { {0,-8}, {4,-8}, {4,0} };
  static SDL_Point digit_8[] = { {0,-8}, {4,-8}, {4,0}, {0,0}, {0,-8}, {0, -4}, {4, -4} };
  static SDL_Point digit_9[] = { {4, 0}, {4,-8}, {0,-8}, {0, -4}, {4, -4} };
  
  static size_t sizes[] = { countof(digit_0), 
                            countof(digit_1), 
                            countof(digit_2), 
                            countof(digit_3), 
                            countof(digit_4), 
                            countof(digit_5), 
                            countof(digit_6), 
                            countof(digit_7), 
                            countof(digit_8), 
                            countof(digit_9) };
  static SDL_Point * digits[] = {digit_0, digit_1, digit_2, digit_3, digit_4, digit_5, digit_6, digit_7, digit_8, digit_9 };

  std::array<SDL_Point, 7> points;
  long long score = game.get_score();
  int no_of_digits = 0;
  if (score > 0) {
    no_of_digits = std::trunc( std::log10( score ) ) + 1;
  }
  size_t x = SCORE_X + 20 * no_of_digits;
  size_t y = SCORE_Y;
  
  // Kleine Status-Markierung links von der Punktzahl.
  SDL_RenderDrawLine(renderer, SCORE_X - 14, SCORE_Y - 8, SCORE_X - 14, SCORE_Y + 8);
  SDL_RenderDrawLine(renderer, SCORE_X - 14, SCORE_Y - 8, SCORE_X - 10, SCORE_Y - 8);

  do {
    int d = score % 10;
    score /= 10;
    size_t size = sizes[d];
    for (size_t i = 0; i < size; i++) {
      points[i].x = x +  4 * (digits[d] + i)->x;
      points[i].y = y +  4 * (digits[d] + i)->y;
    }
    x -= 20;
    setColor(renderer, 0xFF, 0xFF, 0x00);
    SDL_RenderDrawLines(renderer, points.data(), size );
    setColor(renderer, 0xFF, 0xFF, 0xFF);
    no_of_digits--;
  } while (no_of_digits > 0);
}


bool SDL2Renderer::init() {
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
  } else {
    window = SDL_CreateWindow( title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN );
    if( window == nullptr ) {
      std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    } else {
      renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
      if ( renderer == nullptr )  {
        std::cout << "No renderer from window: " <<  SDL_GetError() << std::endl;
      }
      return true;
    }
  }
  return false;
}


void SDL2Renderer::render() {
  SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x10, 0xFF );
  SDL_RenderClear( renderer );

  // Hintergrund-Sterne für mehr Atmosphäre.
  static SDL_Point stars[] = { {120, 80}, {210, 50}, {330, 40}, {520, 90}, {620, 130}, {720, 35}, {840, 70}, {940, 54} };
  setColor(renderer, 0x66, 0x99, 0xFF);
  for (size_t i = 0; i < countof(stars); i++) {
    SDL_RenderDrawPoint(renderer, stars[i].x, stars[i].y);
  }
  
  for (Body2df * body : game.get_physics().get_bodies() ) {
    TypedBody * typed_body = static_cast<TypedBody *>(body);
    auto type = typed_body->get_type();
    if (type == BodyType::spaceship) {
      render( static_cast<Spaceship *>(typed_body) );
    } else if (type == BodyType::torpedo ) {
      render( static_cast<Torpedo *>(typed_body) );
    } else if (type == BodyType::asteroid) {
      render( static_cast<Asteroid *>(typed_body) );
    } else if (type == BodyType::spaceship_debris ) {
      render( static_cast<SpaceshipDebris *>(typed_body) );
    } else if (type == BodyType::debris) {
      render( static_cast<Debris *>(typed_body) );
    } else if (type == BodyType::saucer) {
      render( static_cast<Saucer *>(typed_body) );
    }
  }
  renderFreeShips();
  renderScore();
  SDL_RenderPresent( renderer );
}


void SDL2Renderer::exit() {
  SDL_DestroyWindow( window );
  SDL_Quit();
}

 
