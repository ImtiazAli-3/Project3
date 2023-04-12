#pragma once
#include <engine.h>
#include "player.h"
#include "../playerhit.h"
#include "../enemyhit.h"
#include "../enemy.h"
#include "../ballistic.h"
#include "../melee.h"
#include "../intro.h"
#include <engine/entities/shapes/heightmap.h>
#include "../meleeEffect.h"
#include "engine/entities/bounding_box.h"
class pickup;

class example_layer : public engine::layer
{
public:
    example_layer();
	~example_layer();

    void on_update(const engine::timestep& time_step) override;
    void on_render() override; 
    void on_event(engine::event& event) override;

private:

	engine::ref<engine::skybox>			m_skybox{};

	engine::ref<engine::game_object>	m_terrain{};
	engine::ref<engine::game_object>	m_lamp{};

	engine::ref<engine::game_object>	m_mannequin{};
	engine::ref<engine::game_object>	m_bot{};
	engine::ref<engine::game_object>	m_zombie{};
	engine::ref<engine::game_object>	m_zombie2{};

	engine::ref<engine::game_object>	m_cube{};
	engine::ref<engine::game_object>	m_cube1{};
	engine::ref<engine::game_object>	m_cube2{};
	engine::ref<engine::game_object>	m_cube3{};
	engine::ref<engine::game_object>	m_cube4{};
	engine::ref<engine::game_object>	m_cube5{};
	engine::ref<engine::game_object>	m_cube6{};
	engine::ref<engine::game_object>	m_cube7{};

	engine::ref<engine::game_object>	m_light{};
	engine::ref<engine::game_object>	m_light1{};

	engine::ref<engine::material>		m_material{};
	engine::ref<engine::material>		m_mannequin_material{};
	engine::ref<engine::material>		m_lightsource_material{};
	engine::ref<engine::material>		m_ballistic_material{};
	engine::ref<engine::material>		m_cube_material{};

	player								m_player{};
	enemy								m_enemy{};
	enemy								m_enemy1{};
	enemy								m_enemy2{};

	engine::bounding_box				m_player_box;
	engine::bounding_box				m_bot_box;
	engine::bounding_box				m_zombie_box;
	engine::bounding_box				m_zombie2_box;

	engine::bounding_box				m_cube_box;
	engine::bounding_box				m_cube_box1;
	engine::bounding_box				m_cube_box2;
	engine::bounding_box				m_cube_box3;
	engine::bounding_box				m_cube_box4;
	engine::bounding_box				m_cube_box5;

	engine::bounding_box				m_sphere_box;
	engine::bounding_box				m_melee_box;

	engine::ref<engine::heightmap>		m_heightmap;
	engine::DirectionalLight            m_directionalLight;

	engine::SpotLight					m_spotLight;
	engine::SpotLight					m_spotLight1;
	uint32_t							num_spot_lights = 2;
	
	std::vector<engine::ref<engine::game_object>>     m_game_objects{};

	engine::ref<engine::bullet_manager> m_physics_manager{};
	engine::ref<engine::audio_manager>  m_audio_manager{};
	engine::ref<engine::text_manager>	m_text_manager{};

	engine::ref<playerhit>				m_playerhit{};
	engine::ref<playerhit>				m_playerhit2{};
	engine::ref<enemyhit>				m_enemyhit{};
	engine::ref<intro>					m_intro{};

    engine::orthographic_camera			m_2d_camera; 
    engine::perspective_camera			m_3d_camera;

	engine::ref<pickup>					m_pickup{};
	engine::ref<pickup>					m_pickup1{};
	engine::ref<pickup>					m_pickup2{};
	engine::ref<pickup>					m_pickup3{};
	engine::ref<pickup>					m_pickup4{};

	ballistic							m_ballistic;
	melee								m_melee;
	engine::ref<meleeEffect>			m_melee_effect{};

	float								m_physical_terrain_height = 0.5f;
	float								m_prev_sphere_y_vel = 0.f;
	int									coin_count = 0;
	int									hp_count = 70;
	int									ammo_count = 0;
	int									melee_count = 0;


};
