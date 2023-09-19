#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <iostream>

static const uint32_t NUM_ROWS = 15;

const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t MAXIMUM_ENERGY = 100;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;

const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

enum entity_type_t
{
  empty,
  plant,
  herbivore,
  carnivore
};

enum actions_type_t
{
  eat,
  move,
  reproduction
};

struct pos_t
{
  uint32_t i;
  uint32_t j;
};

struct entity_t
{
  entity_type_t type;
  int32_t energy;
  int32_t age;
};

NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {plant, "P"},
                                                {herbivore, "H"},
                                                {carnivore, "C"},
                                            })

namespace nlohmann
{
  void to_json(nlohmann::json &j, const entity_t &e)
  {
    j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
  }
}

static std::vector<std::vector<entity_t>> entity_grid;
static int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

bool random_action(float probability)
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0);
  return dis(gen) < probability;
}

uint32_t random_integer(const int min, const int max)
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}

uint32_t random_position(const std::vector<int> positions)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distribution(0, positions.size() - 1);
  int randomIndex = distribution(gen);
  return positions[randomIndex];
}

void start_grid(const entity_type_t type, const int initial_quantity)
{
  for (int i = 0; i < initial_quantity; i++)
  {
    uint32_t random_row = random_integer(0, NUM_ROWS - 1);
    uint32_t random_col = random_integer(0, NUM_ROWS - 1);

    if (entity_grid[random_row][random_col].type == empty)
    {
      entity_grid[random_row][random_col].type = type;
      entity_grid[random_row][random_col].energy = MAXIMUM_ENERGY;
      entity_grid[random_row][random_col].age = 0;
    }
    else
    {
      i--;
    }
  }
};

void clean_position(const int i, const int j)
{
  entity_grid[i][j].type = empty;
  entity_grid[i][j].energy = 0;
  entity_grid[i][j].age = 0;
};

std::vector<int> search_positions(const entity_type_t type, const int i, const int j)
{
  std::vector<int> valid_positions;

  for (int k = 0; k < 4; k++)
  {
    if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
    {
      if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == type)
      {
        valid_positions.push_back(k);
      }
    }
  }

  return valid_positions;
}

void next_positions(const std::vector<int> valid_positions, const entity_type_t type, const int i, const int j, const actions_type_t action)
{
  if (valid_positions.size() != 0)
  {
    if (action == eat)
    {
      int random_pos = random_position(valid_positions);
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].type = type;
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].age = entity_grid[i][j].age;
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].energy = entity_grid[i][j].energy + (type == herbivore ? 30 : 20);

      clean_position(i, j);
    }
    if (action == move)
    {
      int random_pos = random_position(valid_positions);
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].type = type;
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].age = entity_grid[i][j].age;
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].energy = entity_grid[i][j].energy - 5;

      clean_position(i, j);
    }
    if (action == reproduction)
    {
      int random_pos = random_position(valid_positions);
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].type = type;
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].age = 0;
      entity_grid[i + directions[random_pos][0]][j + directions[random_pos][1]].energy = MAXIMUM_ENERGY;

      type != plant ? (entity_grid[i][j].energy = entity_grid[i][j].energy - 10) : (entity_grid[i][j].energy = entity_grid[i][j].energy);
    }
  }
}

bool is_death(const entity_type_t type, const int i, const int j)
{
  if ((entity_grid[i][j].age > (type == plant ? PLANT_MAXIMUM_AGE : (type == herbivore ? HERBIVORE_MAXIMUM_AGE : CARNIVORE_MAXIMUM_AGE))) || type != plant && entity_grid[i][j].energy <= 0)
  {
    clean_position(i, j);

    return true;
  }

  return false;
};

int main()
{
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")
  ([](crow::request &, crow::response &res)
   {
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });

  CROW_ROUTE(app, "/start-simulation")
      .methods("POST"_method)([](crow::request &req, crow::response &res)
                              {
            nlohmann::json request_body = nlohmann::json::parse(req.body);

            uint32_t total_entinties = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
            if (total_entinties > NUM_ROWS * NUM_ROWS)
            {
                res.code = 400;
                res.body = "Too many entities";
                res.end();
                return;
            }

            entity_grid.clear();
            entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, {empty, 0, 0}));

            start_grid(plant, (uint32_t)request_body["plants"]);
            start_grid(herbivore, (uint32_t)request_body["herbivores"]);
            start_grid(carnivore, (uint32_t)request_body["carnivores"]);

            nlohmann::json json_grid = entity_grid;
            res.body = json_grid.dump();
            res.end(); });

  CROW_ROUTE(app, "/next-iteration")
      .methods("GET"_method)([]()
                             {
            for (int i = 0; i < NUM_ROWS; i++)
            {
              for (int j = 0; j < NUM_ROWS; j++)
              {
                if (entity_grid[i][j].type != empty && !is_death(entity_grid[i][j].type, i, j))
                {
                  entity_grid[i][j].age++;

                  switch (entity_grid[i][j].type)
                  {
                    case plant:
                      if (random_action(PLANT_REPRODUCTION_PROBABILITY))
                          next_positions(search_positions(empty, i, j), plant, i, j, reproduction);
                      break;
                    case herbivore:
                      if (random_action(HERBIVORE_EAT_PROBABILITY))
                          next_positions(search_positions(plant, i, j), entity_grid[i][j].type, i, j, eat);

                      if (random_action(HERBIVORE_MOVE_PROBABILITY))
                          next_positions(search_positions(empty, i, j), entity_grid[i][j].type, i, j, move);

                      if (random_action(HERBIVORE_REPRODUCTION_PROBABILITY) && entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION)
                          next_positions(search_positions(empty, i, j), entity_grid[i][j].type, i, j, reproduction);
                      break;
                    case carnivore:
                      if (random_action(CARNIVORE_EAT_PROBABILITY))
                          next_positions(search_positions(herbivore, i, j), entity_grid[i][j].type, i, j, eat);

                      if (random_action(CARNIVORE_MOVE_PROBABILITY))
                          next_positions(search_positions(empty, i, j), entity_grid[i][j].type, i, j, move);

                      if (random_action(CARNIVORE_REPRODUCTION_PROBABILITY) && entity_grid[i][j].energy < THRESHOLD_ENERGY_FOR_REPRODUCTION)
                          next_positions(search_positions(empty, i, j), entity_grid[i][j].type, i, j, reproduction);
                      break;
                  }
                }
              }
            }

            nlohmann::json json_grid = entity_grid;
            return json_grid.dump(); });
  app.port(8080).run();

  return 0;
}
