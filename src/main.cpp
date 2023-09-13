#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <iostream>

static const uint32_t NUM_ROWS = 15;

// Constants
const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t MAXIMUM_ENERGY = 100;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;

// Probabilities
const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

// Type definitions
enum entity_type_t
{
    empty,
    plant,
    herbivore,
    carnivore
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

// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {plant, "P"},
                                                {herbivore, "H"},
                                                {carnivore, "C"},
                                            })

// Auxiliary code to convert the entity_t struct to a JSON object
namespace nlohmann
{
    void to_json(nlohmann::json &j, const entity_t &e)
    {
        j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
    }
}

// Grid that contains the entities
static std::vector<std::vector<entity_t>> entity_grid;

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

int main()
{
    crow::SimpleApp app;

    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")
    ([](crow::request &, crow::response &res)
     {
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });

    CROW_ROUTE(app, "/start-simulation")
        .methods("POST"_method)([](crow::request &req, crow::response &res)
                                { 
        // Parse the JSON request body
        nlohmann::json request_body = nlohmann::json::parse(req.body);

       // Validate the request body 
        uint32_t total_entinties = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
        if (total_entinties > NUM_ROWS * NUM_ROWS) {
        res.code = 400;
        res.body = "Too many entities";
        res.end();
        return;
        }

        // Clear the entity grid
        entity_grid.clear();
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0}));
        
        // Create the entities
        // <YOUR CODE HERE>
        // Create plants
        for (int i = 0; i < (uint32_t)request_body["plants"]; i++){
            uint32_t random_row = random_integer(0, NUM_ROWS - 1);
            uint32_t random_col = random_integer(0, NUM_ROWS - 1);

            if (entity_grid[random_row][random_col].type == empty){
                entity_grid[random_row][random_col].type = plant;
                entity_grid[random_row][random_col].energy = 0;
                entity_grid[random_row][random_col].age = 0;
            }
            else{
                i--;
            }
        }

        // Create herbivores
        for (int i = 0; i < (uint32_t)request_body["herbivores"]; i++){
            uint32_t random_row = random_integer(0, NUM_ROWS - 1);
            uint32_t random_col = random_integer(0, NUM_ROWS - 1);

            if (entity_grid[random_row][random_col].type == empty){
                entity_grid[random_row][random_col].type = herbivore;
                entity_grid[random_row][random_col].energy = MAXIMUM_ENERGY;
                entity_grid[random_row][random_col].age = 0;
            }
            else{
                i--;
            }
        }

        // Create carnivores
        for (int i = 0; i < (uint32_t)request_body["carnivores"]; i++){
            uint32_t random_row = random_integer(0, NUM_ROWS - 1);
            uint32_t random_col = random_integer(0, NUM_ROWS - 1);

            if (entity_grid[random_row][random_col].type == empty){
                entity_grid[random_row][random_col].type = carnivore;
                entity_grid[random_row][random_col].energy = MAXIMUM_ENERGY;
                entity_grid[random_row][random_col].age = 0;
            }
            else{
                i--;
            }
        }

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); });

    // Endpoint to process HTTP GET requests for the next simulation iteration
    CROW_ROUTE(app, "/next-iteration")
        .methods("GET"_method)([]()
                               {
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity

        // <YOUR CODE HERE>
        // Iterate over the entity grid and simulate the behaviour of each entity
        for (int i = 0; i < NUM_ROWS; i++)
        {
            for (int j = 0; j < NUM_ROWS; j++)
            {

                if (entity_grid[i][j].type == plant)
                {

                    entity_grid[i][j].age++;

                    if (entity_grid[i][j].age > PLANT_MAXIMUM_AGE)
                    {
                        entity_grid[i][j].type = empty;
                        entity_grid[i][j].energy = 0;
                        entity_grid[i][j].age = 0;
                    }
                    else if (random_action(PLANT_REPRODUCTION_PROBABILITY))
                    {
                        std::vector<int> void_valid_positions;

                        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int k = 0; k < 4; k++)
                        {
                            if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
                            {
                                if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == empty)
                                {
                                    void_valid_positions.push_back(k);
                                }
                            }
                        }

                        if (void_valid_positions.size() != 0)
                        {
                            int random_position_void = random_position(void_valid_positions);
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].type = plant;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].age = 0;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].energy = 0;
                        }
                    }
                }

                if (entity_grid[i][j].type == herbivore)
                {

                    entity_grid[i][j].age++;

                    if (entity_grid[i][j].age > HERBIVORE_MAXIMUM_AGE || entity_grid[i][j].energy <= 0)
                    {
                        entity_grid[i][j].type = empty;
                        entity_grid[i][j].energy = 0;
                        entity_grid[i][j].age = 0;
                    }
                    else if (random_action(HERBIVORE_EAT_PROBABILITY))
                    {

                        std::vector<int> plant_valid_postion;

                        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int k = 0; k < 4; k++)
                        {
                            if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
                            {
                                if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == plant)
                                {
                                    plant_valid_postion.push_back(k);
                                }
                            }
                        }

                        if (plant_valid_postion.size() != 0)
                        {
                            int random_position_void = random_position(plant_valid_postion);
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].type = empty;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].age = 0;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].energy = 0;

                            entity_grid[i][j].energy = entity_grid[i][j].energy + 30;
                        }
                    }
                    else if (random_action(HERBIVORE_MOVE_PROBABILITY))
                    {

                        std::vector<int> void_valid_postion;

                        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int k = 0; k < 4; k++)
                        {
                            if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
                            {
                                if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == empty)
                                {
                                    void_valid_postion.push_back(k);
                                }
                            }
                        }

                        if (void_valid_postion.size() != 0)
                        {
                            int random_position_void = random_position(void_valid_postion);
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].type = herbivore;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].age = entity_grid[i][j].age;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].energy = entity_grid[i][j].energy - 5;

                            entity_grid[i][j].type = empty;
                            entity_grid[i][j].age = 0;
                            entity_grid[i][j].energy = 0;
                        }
                    }
                    else if (random_action(HERBIVORE_REPRODUCTION_PROBABILITY) && entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION)
                    {

                        std::vector<int> void_valid_postion;

                        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int k = 0; k < 4; k++)
                        {
                            if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
                            {
                                if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == empty)
                                {
                                    void_valid_postion.push_back(k);
                                }
                            }
                        }

                        if (void_valid_postion.size() != 0)
                        {
                            int random_position_void = random_position(void_valid_postion);
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].type = herbivore;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].age = 0;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].energy = MAXIMUM_ENERGY;

                            entity_grid[i][j].energy = entity_grid[i][j].energy - 10;
                        }
                    }
                }

                if (entity_grid[i][j].type == carnivore)
                {

                    entity_grid[i][j].age++;

                    if (entity_grid[i][j].age > CARNIVORE_MAXIMUM_AGE || entity_grid[i][j].energy <= 0)
                    {
                        entity_grid[i][j].type = empty;
                        entity_grid[i][j].energy = 0;
                        entity_grid[i][j].age = 0;
                    }
                    else if (random_action(CARNIVORE_EAT_PROBABILITY))
                    {

                        std::vector<int> herbivore_valid_postion;

                        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int k = 0; k < 4; k++)
                        {
                            if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
                            {
                                if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == herbivore)
                                {
                                    herbivore_valid_postion.push_back(k);
                                }
                            }
                        }

                        if (herbivore_valid_postion.size() != 0)
                        {
                            int random_position_void = random_position(herbivore_valid_postion);
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].type = empty;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].age = 0;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].energy = 0;

                            entity_grid[i][j].energy = entity_grid[i][j].energy + 20;
                        }
                    }
                    else if (random_action(CARNIVORE_MOVE_PROBABILITY))
                    {

                        std::vector<int> void_valid_postion;

                        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int k = 0; k < 4; k++)
                        {
                            if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
                            {
                                if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == empty)
                                {
                                    void_valid_postion.push_back(k);
                                }
                            }
                        }

                        if (void_valid_postion.size() != 0)
                        {
                            int random_position_void = random_position(void_valid_postion);
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].type = carnivore;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].age = entity_grid[i][j].age;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].energy = entity_grid[i][j].energy - 5;

                            entity_grid[i][j].type = empty;
                            entity_grid[i][j].age = 0;
                            entity_grid[i][j].energy = 0;
                        }
                    }
                    else if (random_action(CARNIVORE_REPRODUCTION_PROBABILITY) && entity_grid[i][j].energy < THRESHOLD_ENERGY_FOR_REPRODUCTION)
                    {

                        std::vector<int> void_valid_postion;

                        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int k = 0; k < 4; k++)
                        {
                            if (i + directions[k][0] >= 0 && i + directions[k][0] < NUM_ROWS && j + directions[k][1] >= 0 && j + directions[k][1] < NUM_ROWS)
                            {
                                if (entity_grid[i + directions[k][0]][j + directions[k][1]].type == empty)
                                {
                                    void_valid_postion.push_back(k);
                                }
                            }
                        }

                        if (void_valid_postion.size() != 0)
                        {
                            int random_position_void = random_position(void_valid_postion);
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].type = carnivore;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].age = 0;
                            entity_grid[i + directions[random_position_void][0]][j + directions[random_position_void][1]].energy = MAXIMUM_ENERGY;

                            entity_grid[i][j].energy = entity_grid[i][j].energy - 10;
                        }
                    }
                }
            }
            // Return the JSON representation of the entity grid
            
        }
        
        nlohmann::json json_grid = entity_grid;
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}