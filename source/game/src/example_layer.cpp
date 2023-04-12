#include "example_layer.h"
#include "platform/opengl/gl_shader.h"

#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
#include "engine/events/key_event.h"
#include "engine/utils/track.h"
#include "../pickup.h"

example_layer::example_layer() 
    :m_2d_camera(-1.6f, 1.6f, -0.9f, 0.9f), 
    m_3d_camera((float)engine::application::window().width(), (float)engine::application::window().height())
{

	// Hide the mouse and lock it inside the window
    //engine::input::anchor_mouse(true);
    engine::application::window().hide_mouse_cursor();
	// Initialise audio and play background music, initialise sound effects
	m_audio_manager = engine::audio_manager::instance();
	m_audio_manager->init();
	m_audio_manager->load_sound("assets/audio/bounce.wav", engine::sound_type::spatialised, "bounce"); // Royalty free sound from https://mixkit.co/free-sound-effects/game/
	m_audio_manager->load_sound("assets/audio/pickup.wav", engine::sound_type::spatialised, "pickup"); // Royalty free sound from https://mixkit.co/free-sound-effects/game/
	m_audio_manager->load_sound("assets/audio/shot.mp3", engine::sound_type::spatialised, "shot"); // Royalty free sound from https://pixabay.com/sound-effects/search/gunshot/
	m_audio_manager->load_sound("assets/audio/weaponpick.wav", engine::sound_type::spatialised, "weaponpick"); // Royalty free sound from https://mixkit.co/free-sound-effects/game/
	m_audio_manager->load_sound("assets/audio/coinpick.wav", engine::sound_type::spatialised, "coinpick"); // Royalty free sound from https://mixkit.co/free-sound-effects/game/d
	m_audio_manager->load_sound("assets/audio/playerhit.wav", engine::sound_type::spatialised, "playerhit"); // Royalty free sound from https://mixkit.co/free-sound-effects/game/d
	m_audio_manager->load_sound("assets/audio/DST-impuretechnology.mp3", engine::sound_type::track, "music");  // Royalty free music from http://www.nosoapradio.us/ //MUSIC IS LOUD
	//m_audio_manager->play("music");

	// Initialise the shaders, materials and lights
	auto mesh_shader = engine::renderer::shaders_library()->get("mesh");
	auto text_shader = engine::renderer::shaders_library()->get("text_2D");
	//add a directional light source
	m_directionalLight.Color = glm::vec3(1.0f, 1.0f, 1.0f);
	m_directionalLight.AmbientIntensity = 0.25f;
	m_directionalLight.DiffuseIntensity = 0.6f;
	m_directionalLight.Direction = glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f));
	//adding spotlight for lighting effects
	m_spotLight.Color = glm::vec3(1.0f, 1.0f, 0.5f);
	m_spotLight.AmbientIntensity = 1.f;
	m_spotLight.DiffuseIntensity = 0.6f;
	m_spotLight.Position = glm::vec3(7.0f, 4.25f, 25.0f);
	m_spotLight.Direction = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
	m_spotLight.Cutoff = 0.00001f;
	m_spotLight.Attenuation.Constant = 1.0f;
	m_spotLight.Attenuation.Linear = 0.1f;
	m_spotLight.Attenuation.Exp = 0.01f;
	//adding the second spotlight 
	m_spotLight1.Color = glm::vec3(1.0f, 1.0f, 0.5f);
	m_spotLight1.AmbientIntensity = 1.f;
	m_spotLight1.DiffuseIntensity = 0.6f;
	m_spotLight1.Position = glm::vec3(-7.0f, 4.25f, 15.0f);
	m_spotLight1.Direction = glm::normalize(glm::vec3(0.f, -1.0f, 0.f));
	m_spotLight1.Cutoff = 0.5f;
	m_spotLight1.Attenuation.Constant = 1.0f;
	m_spotLight1.Attenuation.Linear = 0.1f;
	m_spotLight1.Attenuation.Exp = 0.01f;

	// set color texture unit
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->bind();
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("lighting_on", true);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gColorMap", 0);
	m_directionalLight.submit(mesh_shader);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gMatSpecularIntensity", 1.f);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gSpecularPower", 10.f);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("transparency", 1.0f);

	std::dynamic_pointer_cast<engine::gl_shader>(text_shader)->bind();
	std::dynamic_pointer_cast<engine::gl_shader>(text_shader)->set_uniform("projection",
		glm::ortho(0.f, (float)engine::application::window().width(), 0.f,
		(float)engine::application::window().height()));
	//set material settings
	m_material = engine::material::create(1.0f, glm::vec3(1.0f, 0.1f, 0.07f),
		glm::vec3(1.0f, 0.1f, 0.07f), glm::vec3(0.5f, 0.5f, 0.5f), 1.0f);
	m_ballistic_material = engine::material::create(1.f, glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);
	m_mannequin_material = engine::material::create(1.0f, glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), 1.0f);

	m_melee_effect = meleeEffect::create(glm::vec3(1.f, 0.f, 1.0f), glm::vec3(0.f, 0.f, 1.f), 3.0f);

	//add city skybox
	// Skybox texture from https://tools.wwwtyro.net/space-3d/index.html#animationSpeed=0&fov=39&nebulae=true&pointStars=true&resolution=4096&seed=7imxih8ac6g&stars=true&sun=true
	m_skybox = engine::skybox::create(50.f,
		{ engine::texture_2d::create("assets/skybox1/front.png", true),
		  engine::texture_2d::create("assets/skybox1/right.png", true),
		  engine::texture_2d::create("assets/skybox1/back.png", true),
		  engine::texture_2d::create("assets/skybox1/left.png", true),
		  engine::texture_2d::create("assets/skybox1/top.png", true),
		  engine::texture_2d::create("assets/skybox1/bottom.png", true)
		});
	//using player model from tutorial,
	engine::ref<engine::skinned_mesh> m_skinned_mesh = engine::skinned_mesh::create("assets/models/animated/mannequin/free3Dmodel.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/walking.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/idle.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/jump.dae");
	m_skinned_mesh->LoadAnimationFile("assets/models/animated/mannequin/standard_run.dae");
	m_skinned_mesh->switch_root_movement(false);
	// Load the player model. Create a player object. Set its properties
	engine::game_object_properties mannequin_props;
	mannequin_props.animated_mesh = m_skinned_mesh;
	mannequin_props.scale = glm::vec3(1.5f/ glm::max(m_skinned_mesh->size().x, glm::max(m_skinned_mesh->size().y, m_skinned_mesh->size().z)));
	mannequin_props.position = glm::vec3(3.0f, 0.5f, -5.0f);
	mannequin_props.type = 0;
	mannequin_props.bounding_shape = glm::vec3(m_skinned_mesh->size().x / 2.5f,
		m_skinned_mesh->size().y, m_skinned_mesh->size().x / 2.5f);
	m_mannequin = engine::game_object::create(mannequin_props);
	m_player.initialise(m_mannequin);

	m_player_box.set_box(mannequin_props.bounding_shape.x *
		mannequin_props.scale.x, mannequin_props.bounding_shape.y *
		mannequin_props.scale.x, mannequin_props.bounding_shape.z *
		mannequin_props.scale.x, mannequin_props.position);

	// Load the terrain with a heightmap. Set its properties
	m_heightmap = engine::heightmap::create("assets/textures/heightmap.bmp", "assets/textures/terrain.png", 100.f, 100.f, glm::vec3(0.f, 0.6f, 0.f), 10.f);
	engine::game_object_properties terrain_props;
	terrain_props.meshes = { m_heightmap->mesh() };
	terrain_props.textures = { m_heightmap->texture() };
	terrain_props.is_static = true;
	terrain_props.type = 0;
	terrain_props.bounding_shape = glm::vec3(m_heightmap->terrain_size().x, m_physical_terrain_height, m_heightmap->terrain_size().y);
	terrain_props.restitution = 0.92f;
	m_terrain = engine::game_object::create(terrain_props);
	// Load the spotlights. Create the objects and set its properties
	engine::ref<engine::sphere> light_shape = engine::sphere::create(20, 40, 0.05f);
	engine::game_object_properties light_props;
	light_props.position = { -2.5f, 0.0f, -2.5f };
	light_props.meshes = { light_shape->mesh() };
	light_props.type = 1;
	light_props.bounding_shape = glm::vec3(0.5f);
	light_props.restitution = 0.92f;
	light_props.mass = 0.000001f;
	light_props.bounding_shape = glm::vec3(0.1f);
	m_light = engine::game_object::create(light_props);
	m_light1 = engine::game_object::create(light_props);

	m_sphere_box.set_box(light_props.bounding_shape.x, light_props.bounding_shape.y, light_props.bounding_shape.z, light_props.position);

	m_melee_box.set_box(light_props.bounding_shape.x*10.f, light_props.bounding_shape.y*10.f, light_props.bounding_shape.z*10.f, light_props.position);

#// Load the bot model. Create a bot  object. Set its properties
	//set a bounding box to check for collisions
	engine::ref <engine::model> bot_model = engine::model::create("assets/zombie/source/ZombieStatic.fbx");
	engine::ref<engine::texture_2d> bot_texture = engine::texture_2d::create("assets/zombie/textures/@Diffuse_3.png", true);
	engine::game_object_properties bot_props;
	bot_props.meshes = bot_model->meshes();
	bot_props.textures = bot_model->textures();
	float bot_scale = 1.f;
	bot_props.position = { 4.f,0.6f, 15.f };
	bot_props.scale = glm::vec3(bot_scale);
	bot_props.textures = { bot_texture };
	bot_props.bounding_shape = bot_model->size();
	m_bot = engine::game_object::create(bot_props);
	m_enemy.initialise(m_bot, bot_props.position, glm::vec3(0.f, 0.f, 4.f));
	m_bot->set_offset(bot_model->offset());
	m_bot_box.set_box(bot_props.bounding_shape.x*0.8f,bot_props.bounding_shape.y* 1.f, bot_props.bounding_shape.z*0.6f, bot_props.position);

#// Load the zombie model. Set its properties
	//set a bounding box to check for collisions
	engine::ref <engine::model> zom_model = engine::model::create("assets/zombie1/source/z1.fbx");
	engine::ref<engine::texture_2d> zom_texture = engine::texture_2d::create("assets/zombie1/textures/z1.png", true);
	engine::game_object_properties zom_props;
	zom_props.meshes = zom_model->meshes();
	zom_props.textures = zom_model->textures();
	float zom_scale = 0.012f;
	zom_props.position = { -14.f,0.6f, 16.5f };
	zom_props.scale = glm::vec3(zom_scale);
	zom_props.textures = { zom_texture };
	zom_props.bounding_shape = zom_model->size();
	m_zombie = engine::game_object::create(zom_props);
	m_enemy1.initialise(m_zombie, zom_props.position, glm::vec3(1.f, 0.f, 0.f));
	m_zombie->set_offset(zom_model->offset());
	m_zombie_box.set_box(zom_props.bounding_shape.x * 0.01f, zom_props.bounding_shape.y * 0.01f, zom_props.bounding_shape.z * 0.008f, zom_props.position);

#// Load the zombie model. Set its properties with some texture
	//set a bounding box to check for collisions
	engine::ref <engine::model> zombie2_model = engine::model::create("assets/zombie2/source/zombie1.fbx");
	engine::ref<engine::texture_2d> zombie2_texture = engine::texture_2d::create("assets/zombie2/textures/z1.png", true);
	engine::game_object_properties zombie2_props;
	zombie2_props.meshes = zombie2_model->meshes();
	zombie2_props.textures = zombie2_model->textures();
	float z2_scale = 0.01f;
	zombie2_props.position = { -2.5f,0.5f, 10.f };
	zombie2_props.scale = glm::vec3(z2_scale);
	zombie2_props.textures = { zombie2_texture };
	zombie2_props.bounding_shape = zombie2_model->size() / 2.f * z2_scale;
	zombie2_props.bounding_shape = zom_model->size();
	m_zombie2 = engine::game_object::create(zombie2_props);
	m_enemy2.initialise(m_zombie2, zombie2_props.position, glm::vec3(1.f, 0.f, 1.f));
	m_zombie2->set_offset(zombie2_model->offset());
	m_zombie2_box.set_box(zombie2_props.bounding_shape.x * 0.01f, zombie2_props.bounding_shape.y * 0.01f, zombie2_props.bounding_shape.z * 0.008f, zombie2_props.position);
	//the objects created above are enemies that have bounding boxes that collide with other bounding boxes
	//different things happen with the different type of box hit


	//initialise ballistic and melee
	m_ballistic.initialise(engine::game_object::create(light_props));
	m_melee.initialise(engine::game_object::create(light_props));

#// Load the knife pickup model, Create the object. Set its properties
	engine::ref <engine::model> knife_model = engine::model::create("assets/Knife/knife.fbx");
	engine::game_object_properties knife_props;
	knife_props.meshes = knife_model->meshes();
	knife_props.textures = knife_model->textures();
	float knife_scale = 0.05f;
	knife_props.position = { -2.f,1.f, 26.f };
	knife_props.scale = glm::vec3(knife_scale);
	knife_props.bounding_shape = knife_model->size() / knife_scale;
	m_pickup1 = pickup::create(knife_props);
	m_pickup1->init();
#// Load the gun pickup model. Create a gun pickup object. Set its properties
	engine::ref <engine::model> gun_model = engine::model::create("assets/Weapons/AWP.fbx");
	engine::game_object_properties gun_props;
	gun_props.meshes = gun_model->meshes();
	gun_props.textures = gun_model->textures();
	float gun_scale = 0.02f;
	gun_props.position = { 7.f, 1.f, 26.f };
	gun_props.scale = glm::vec3(gun_scale);
	gun_props.bounding_shape = gun_model->size() / gun_scale;
	m_pickup2 = pickup::create(gun_props);
	m_pickup2->init();
	
#// Load the medkit pickup model. Create a medkit pickup object. Set its properties
	engine::ref <engine::model> med_model = engine::model::create("assets/health-pick-up-v1/source/health.fbx");
	engine::game_object_properties med_props;
	med_props.meshes = med_model->meshes();
	med_props.textures = med_model->textures();
	float med_scale = 0.2f;
	med_props.position = { 5.f,1.f, 25.f };
	med_props.scale = glm::vec3(med_scale);
	med_props.bounding_shape = med_model->size() / med_scale;
	m_pickup = pickup::create(med_props);
	m_pickup->init();

#// Load the ammo pickup models. Create coin pickup objects. Set the properties
	//the coin was switched to an ammo box, the rest of the code was not changed due to time and ease ( i was lazy :/ )
	engine::ref <engine::model> coin_model = engine::model::create("assets/ammo/source/ammo.fbx");
	engine::ref<engine::texture_2d> coin_texture = engine::texture_2d::create("assets/ammo/textures/ammo.jpeg", true);
	engine::game_object_properties coin_props;
	coin_props.meshes = coin_model->meshes();
	coin_props.textures = coin_model->textures();
	float coin_scale = 0.2f;
	coin_props.position = { -13.f,1.f, 16.f };
	coin_props.bounding_shape = coin_model->size() / coin_scale;
	coin_props.scale = glm::vec3(coin_scale);
	coin_props.textures = {coin_texture};
	m_pickup3 = pickup::create(coin_props);
	m_pickup3->init();

	engine::ref <engine::model> coin_model1 = engine::model::create("assets/coin/source/Coin.fbx");
	engine::ref<engine::texture_2d> coin_texture1 = engine::texture_2d::create("assets/coin/textures/Coin2_BaseColor.jpg", true);
	engine::game_object_properties coin_props1;
	coin_props1.meshes = coin_model1->meshes();
	coin_props1.textures = coin_model1->textures();
	float coin_scale1 = 0.6f;
	coin_props1.position = { -6.f, 0.6f, 21.f };
	coin_props1.bounding_shape = coin_model1->size() / coin_scale1;
	coin_props1.scale = glm::vec3(coin_scale1);
	coin_props1.textures = { coin_texture1 };
	m_pickup4 = pickup::create(coin_props1);
	m_pickup4->init();

#// Load the lamp models. Create lamp object. Set its properties
	engine::ref <engine::model> lamp_model = engine::model::create("assets/StreetLamp/StreetLamp.3ds");
	engine::game_object_properties lamp_props;
	lamp_props.meshes = lamp_model->meshes();
	lamp_props.textures = lamp_model->textures();
	float lamp_scale = 4.5f / glm::max(lamp_model->size().x, glm::max(lamp_model->size().y, lamp_model->size().z));
	lamp_props.position = { 7.f, 2.f, -5.f };
	lamp_props.bounding_shape = lamp_model->size() / 2.f * lamp_scale;
	lamp_props.scale = glm::vec3(lamp_scale);
	m_lamp = engine::game_object::create(lamp_props);

#// this block of code creates a few cubes to imitate buildings.
	//textures are then added to make it seem like an actual block of buildings
	//the buildings all have AABB which stop the player and enemy from walking into it
	engine::ref<engine::cuboid> cube_shape = engine::cuboid::create(glm::vec3(2.5f), false);
	engine::ref<engine::texture_2d> cube_texture = engine::texture_2d::create("assets/textures/shop1.jpg", true);
	engine::game_object_properties cube_props;
	cube_props.position = { 12.5f, 3.f, 27.5f };
	cube_props.meshes = { cube_shape->mesh() };
	cube_props.textures = { cube_texture };
	cube_props.bounding_shape = glm::vec3(1.f);
	m_cube = engine::game_object::create(cube_props);
	m_cube_box.set_box(cube_props.bounding_shape.x*6.f, cube_props.bounding_shape.y*6.f, cube_props.bounding_shape.z*11.f, glm::vec3(13.f, 0.5f, 24.5f));

	engine::ref<engine::cuboid> cube_shape1 = engine::cuboid::create(glm::vec3(3.f), false);
	engine::ref<engine::texture_2d> cube_texture1 = engine::texture_2d::create("assets/textures/shop3.jpg", true);
	engine::game_object_properties cube_props1;
	cube_props1.position = { 13.f, 3.5f, 22.f };
	cube_props1.meshes = { cube_shape1->mesh() };
	cube_props1.textures = { cube_texture1 };
	m_cube1 = engine::game_object::create(cube_props1);

	engine::ref<engine::cuboid> cube_shape2 = engine::cuboid::create(glm::vec3(3.f), false);
	engine::ref<engine::texture_2d> cube_texture2 = engine::texture_2d::create("assets/textures/shop4.jpg", true);
	engine::game_object_properties cube_props2;
	cube_props2.position = { -13.f, 3.5f, 28.f };
	cube_props2.meshes = { cube_shape2->mesh() };
	cube_props2.textures = { cube_texture2 };
	cube_props2.bounding_shape = glm::vec3(1.f);
	m_cube2 = engine::game_object::create(cube_props2);
	m_cube_box1.set_box(cube_props.bounding_shape.x * 6.f, cube_props.bounding_shape.y * 6.f, cube_props.bounding_shape.z * 12.f, glm::vec3(-13.f, 0.5f, 25.f));

	engine::ref<engine::cuboid> cube_shape3 = engine::cuboid::create(glm::vec3(3.f), false);
	engine::ref<engine::texture_2d> cube_texture3 = engine::texture_2d::create("assets/textures/shop2.jfif", true);
	engine::game_object_properties cube_props3;
	cube_props3.position = { -13.f, 3.5f, 22.f };
	cube_props3.meshes = { cube_shape3->mesh() };
	cube_props3.textures = { cube_texture3 };
	m_cube3 = engine::game_object::create(cube_props3);

	engine::ref<engine::cuboid> cube_shape4 = engine::cuboid::create(glm::vec3(3.f, 3.f, 6.f), false);
	engine::ref<engine::texture_2d> cube_texture4 = engine::texture_2d::create("assets/textures/shop7.jpg", true);
	engine::game_object_properties cube_props4;
	cube_props4.position = { 13.f, 3.5f, 8.f };
	cube_props4.meshes = { cube_shape4->mesh() };
	cube_props4.textures = { cube_texture4 };
	cube_props4.bounding_shape = glm::vec3(1.f);
	m_cube4 = engine::game_object::create(cube_props4);
	m_cube_box2.set_box(cube_props4.bounding_shape.x * 6.f, cube_props4.bounding_shape.y * 6.f, cube_props4.bounding_shape.z * 12.f, glm::vec3(13.f, 0.5f, 8.f));

	engine::ref<engine::cuboid> cube_shape6 = engine::cuboid::create(glm::vec3(3.f,3.f,6.f), false);
	engine::ref<engine::texture_2d> cube_texture6 = engine::texture_2d::create("assets/textures/shop6.jpg", true);
	engine::game_object_properties cube_props6;
	cube_props6.position = { -13.f, 3.5f, 8.f };
	cube_props6.meshes = { cube_shape6->mesh() };
	cube_props6.textures = { cube_texture6 };
	cube_props6.bounding_shape = glm::vec3(1.f);
	m_cube6 = engine::game_object::create(cube_props6);
	m_cube_box3.set_box(cube_props6.bounding_shape.x * 6.f, cube_props6.bounding_shape.y * 6.f, cube_props6.bounding_shape.z * 12.f, glm::vec3(-13.f, 0.5f, 8.f));

	engine::ref<engine::cuboid> cube_shape5 = engine::cuboid::create(glm::vec3(0.1f, 3.f, 3.f), false);
	engine::ref<engine::texture_2d> cube_texture5 = engine::texture_2d::create("assets/textures/wall.jpg", true);
	engine::game_object_properties cube_props5;
	cube_props5.position = { -16.f, 3.f, 16.5f };
	cube_props5.meshes = { cube_shape5->mesh() };
	cube_props5.textures = { cube_texture5 };
	cube_props5.bounding_shape = glm::vec3(1.f);
	m_cube5 = engine::game_object::create(cube_props5);
	m_cube_box4.set_box(cube_props5.bounding_shape.x*0.1f, cube_props5.bounding_shape.y*6.f, cube_props5.bounding_shape.z*6.f, glm::vec3(-15.5f, 0.5f, 16.5f));

	engine::ref<engine::cuboid> cube_shape7 = engine::cuboid::create(glm::vec3(0.1f, 3.f, 3.f), false);
	engine::ref<engine::texture_2d> cube_texture7 = engine::texture_2d::create("assets/textures/wall.jpg", true);
	engine::game_object_properties cube_props7;
	cube_props7.position = { 16.f, 3.f, 16.5f };
	cube_props7.meshes = { cube_shape7->mesh() };
	cube_props7.textures = { cube_texture7 };
	cube_props7.bounding_shape = glm::vec3(1.f);
	m_cube7 = engine::game_object::create(cube_props7);
	m_cube_box5.set_box(cube_props7.bounding_shape.x*0.1f, cube_props7.bounding_shape.y*6.f, cube_props7.bounding_shape.z*6.f, glm::vec3(15.5f, 0.5f, 16.5f));

	//end of buildings

	m_game_objects.push_back(m_terrain);
	m_physics_manager = engine::bullet_manager::create(m_game_objects);

	//adding the effects for health decrement or increment and the intro screen
	m_enemyhit = enemyhit::create("assets/hit.tga", 4, 6, 16);
	m_playerhit = playerhit::create("assets/red.bmp", 1.0f, (float)engine::application::window().width(), (float)engine::application::window().height());
	m_playerhit2 = playerhit::create("assets/green.bmp", 1.0f, (float)engine::application::window().width(), (float)engine::application::window().height());
	m_intro = intro::create("assets/intro.png", 2.0f, 1.6f, 0.9f);

	m_text_manager = engine::text_manager::create();

	m_skinned_mesh->switch_animation(1);
}

example_layer::~example_layer() {}

void example_layer::on_update(const engine::timestep& time_step) 
{
	//setting the positions of the objects to a variable that is always updated to keep track of it
	glm::vec3 pos = m_player.object()->position();
	glm::vec3 enemy_pos = m_enemy.object()->position();
	glm::vec3 enemy_pos1 = m_enemy1.object()->position();
	glm::vec3 enemy_pos2 = m_enemy2.object()->position();
	glm::vec3 bullet_pos = m_ballistic.object()->position();
	glm::vec3 melee_pos = m_melee.object()->position();

	//the function is updated
	m_playerhit->on_update(time_step);
	m_playerhit2->on_update(time_step);
	m_enemyhit->on_update(time_step);
	m_intro->on_update(time_step);

	//this block of code updates the objects position to keep track of it
	m_player.on_update(time_step);
	m_player_box.on_update(m_player.object()->position());

	m_enemy.on_update(time_step, m_player.object()->position());
	m_bot_box.on_update(m_enemy.object()->position());

	m_enemy1.on_update(time_step, m_player.object()->position());
	m_zombie_box.on_update(m_enemy1.object()->position());

	m_enemy2.on_update(time_step, m_player.object()->position());
	m_zombie2_box.on_update(m_enemy2.object()->position());

	m_sphere_box.on_update(m_ballistic.object()->position());
	m_melee_box.on_update(m_melee.object()->position());

	//these are bounding box collision detectors
	//different boxes have different reactions
	//if a player hits an enemy they lose health. health is decremented with a count
	//if a player weapon hits an enemy, the enemy dies
	//if a player of enemy hits a building, they cannot pass through

	if (m_bot_box.collision(m_player_box)) {
		m_player.object()->set_position(pos);
		m_enemy.object()->set_position(glm::vec3(0.f,-100.f,-100.f));
		m_audio_manager->play_spatialised_sound("playerhit", m_3d_camera.position(),pos);
		m_playerhit->activate();
		hp_count -= 25;
	}
	if (m_zombie_box.collision(m_player_box)) {
		m_player.object()->set_position(pos);
		m_enemy1.object()->set_position(glm::vec3(0.f,-100.f,-100.f));
		m_audio_manager->play_spatialised_sound("playerhit", m_3d_camera.position(),pos);
		m_playerhit->activate();
		hp_count -= 25;
	}
	if (m_zombie2_box.collision(m_player_box)) {
		m_player.object()->set_position(pos);
		m_enemy2.object()->set_position(glm::vec3(0.f,-100.f,-100.f));
		m_audio_manager->play_spatialised_sound("playerhit", m_3d_camera.position(),pos);
		m_playerhit->activate();
		hp_count -= 25;
	}

	if (m_cube_box.collision(m_player_box)|| m_cube_box1.collision(m_player_box) ||
		m_cube_box2.collision(m_player_box) || m_cube_box3.collision(m_player_box) ||
		m_cube_box4.collision(m_player_box) || m_cube_box5.collision(m_player_box)) {
		m_player.object()->set_position(pos);
	}
	if (m_cube_box.collision(m_bot_box)|| m_cube_box1.collision(m_bot_box) ||
		m_cube_box2.collision(m_bot_box) || m_cube_box3.collision(m_bot_box)||
		m_cube_box4.collision(m_bot_box) || m_cube_box5.collision(m_bot_box)) {
		m_enemy.object()->set_position(enemy_pos);
	}
	if (m_cube_box.collision(m_zombie_box)|| m_cube_box1.collision(m_zombie_box) ||
		m_cube_box2.collision(m_zombie_box) || m_cube_box3.collision(m_zombie_box)||
		m_cube_box4.collision(m_zombie_box) || m_cube_box5.collision(m_zombie_box)) {
		m_enemy1.object()->set_position(enemy_pos1);
	}
	if (m_cube_box.collision(m_zombie2_box)|| m_cube_box1.collision(m_zombie2_box) ||
		m_cube_box2.collision(m_zombie2_box) || m_cube_box3.collision(m_zombie2_box)||
		m_cube_box4.collision(m_zombie2_box) || m_cube_box5.collision(m_zombie2_box)) {
		m_enemy2.object()->set_position(enemy_pos2);
	}

	if (m_sphere_box.collision(m_bot_box) || m_melee_box.collision(m_bot_box)) {
		m_enemyhit->activate(glm::vec3(enemy_pos.x,enemy_pos.y+0.5f,enemy_pos.z), 1.f, 1.f);
		m_enemy.object()->set_position(glm::vec3(0.f, -100.f, 0.f));
	}

	if (m_sphere_box.collision(m_zombie_box) || m_melee_box.collision(m_zombie_box)) {
		m_enemyhit->activate(glm::vec3(enemy_pos1.x,enemy_pos1.y+0.5f,enemy_pos1.z), 1.f, 1.f);
		m_enemy1.object()->set_position(glm::vec3(0.f, -100.f, 0.f));
	}
	if (m_sphere_box.collision(m_zombie2_box) || m_melee_box.collision(m_zombie2_box)) {
		m_enemyhit->activate(glm::vec3(enemy_pos2.x,enemy_pos2.y+0.5f,enemy_pos2.z), 1.f, 1.f);
		m_enemy2.object()->set_position(glm::vec3(0.f, -100.f, 0.f));
	}
    //m_3d_camera.on_update(time_step);
	m_physics_manager->dynamics_world_update(m_game_objects, double(time_step));

	m_pickup->update(m_3d_camera.position(), time_step);
	m_pickup1->update(m_3d_camera.position(), time_step);
	m_pickup2->update(m_3d_camera.position(), time_step);
	m_pickup3->update(m_3d_camera.position(), time_step);
	m_pickup4->update(m_3d_camera.position(), time_step);
	m_ballistic.on_update(time_step);
	m_melee.on_update(time_step);

	m_player.update_camera(m_3d_camera);

	m_audio_manager->update_with_camera(m_3d_camera);

	m_melee_effect->on_update(time_step);

} 

void example_layer::on_render() 
{
    engine::render_command::clear_color({0.2f, 0.3f, 0.3f, 1.0f}); 
    engine::render_command::clear();

	// Set up  shader. (renders textures and materials)
	const auto mesh_shader = engine::renderer::shaders_library()->get("mesh");
	engine::renderer::begin_scene(m_3d_camera, mesh_shader);
	//code to check the bounding box position

	//m_player_box.on_render(2.5f, 0.f, 0.f, mesh_shader);
	//m_zombie_box.on_render(2.5f, 0.f, 0.f, mesh_shader);
	//m_cube_box2.on_render(2.5f, 0.f, 0.f, mesh_shader);
	//m_cube_box4.on_render(2.5f, 0.f, 0.f, mesh_shader);
	//m_melee_box.on_render(2.5f, 0.f, 0.f, mesh_shader);

	// Set up some of the scene's parameters in the shader
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gEyeWorldPos", m_3d_camera.position());
	// Position the skybox centred on the player and render it
	glm::mat4 skybox_tranform(1.0f);
	skybox_tranform = glm::translate(skybox_tranform, m_3d_camera.position());
	for (const auto& texture : m_skybox->textures())
	{
		texture->bind();
	}
	engine::renderer::submit(mesh_shader, m_skybox, skybox_tranform);

	engine::renderer::submit(mesh_shader, m_terrain);

	//create the lamps with loop
	for (int x = 0; x <= 7; x++) {
		glm::mat4 lamp_transform(1.0f);
		lamp_transform = glm::translate(lamp_transform, glm::vec3(7.f, 1.f, -5.0f+x*5.f));
		lamp_transform = glm::rotate(lamp_transform, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.f));
		lamp_transform = glm::scale(lamp_transform, m_lamp->scale());
		engine::renderer::submit(mesh_shader, lamp_transform, m_lamp);
	}
		for (int x = 0; x <= 7; x++) {
		glm::mat4 lamp_transform(1.0f);
		lamp_transform = glm::translate(lamp_transform, glm::vec3(-7.f, 1.f, -5.0f+x*5.f));
		lamp_transform = glm::rotate(lamp_transform, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.f));
		lamp_transform = glm::scale(lamp_transform, m_lamp->scale());
		engine::renderer::submit(mesh_shader, lamp_transform, m_lamp);
	}
		//create the gun pickup using the pickup class playing a sound when it gets picked up
	if (m_pickup2->active()) {
		glm::mat4 gun_transform(1.0f);
		gun_transform = glm::translate(gun_transform, m_pickup2->position());
		gun_transform = glm::rotate(gun_transform, m_pickup2->rotation_amount(), glm::vec3(0.0f, 5.0f, 0.f));
		gun_transform = glm::scale(gun_transform, m_pickup2->scale());
		engine::renderer::submit(mesh_shader, gun_transform, m_pickup2);
		glm::vec3 d = m_pickup2->position() - m_player.object()->position();
		if (glm::length(d) <1.f) {
			m_audio_manager->play_spatialised_sound("weaponpick", m_3d_camera.position(), glm::vec3(m_pickup2->position().x, 0.f, m_pickup2->position().z));
			m_pickup2->set_position(glm::vec3(0.f, -100.f, 0.f));
			ammo_count += 3;
		}
	}
	//the change of position places the object at -100 in the y axis essentially removing them from the terrain but not the game wor;d
	// this is an error i faced
	//create the knife pickup using the pickup class playing a sound when it gets picked up
	if (m_pickup1->active()) {
		glm::mat4 knife_transform(1.0f);
		knife_transform = glm::translate(knife_transform, m_pickup1->position());
		knife_transform = glm::rotate(knife_transform, m_pickup1->rotation_amount(), glm::vec3(0.0f, 5.f, 0.f));
		knife_transform = glm::scale(knife_transform, m_pickup1->scale());
		engine::renderer::submit(mesh_shader, knife_transform, m_pickup1);
		glm::vec3 d = m_pickup1->position() - m_player.object()->position();
		if (glm::length(d) <1.f) {
			m_audio_manager->play_spatialised_sound("weaponpick", m_3d_camera.position(), glm::vec3(m_pickup1->position().x, 0.f, m_pickup1->position().z));
			m_pickup1->set_position(glm::vec3(0.f, -100.f, 0.f));
			melee_count = 1;
		}
	}
	//create the medkit pickup using the pickup class playing a different sound when it gets picked up and incrementing the hp count
	if (m_pickup->active()) {
		glm::mat4 med_transform(1.0f);
		med_transform = glm::translate(med_transform, m_pickup->position());
		med_transform = glm::rotate(med_transform, m_pickup->rotation_amount(), glm::vec3(0.f,5.f,0.f));
		med_transform = glm::scale(med_transform, m_pickup->scale());
		engine::renderer::submit(mesh_shader, med_transform, m_pickup);
		glm::vec3 d = m_pickup->position() - m_player.object()->position();
		if (glm::length(d) < 1.f) {
			m_audio_manager->play_spatialised_sound("pickup", m_3d_camera.position(), glm::vec3(m_pickup->position().x, 0.f, m_pickup->position().z));
			hp_count = hp_count + 30;
			m_pickup->set_position(glm::vec3(0.f, -100.f, 0.f));
			m_playerhit2->activate(); //increase the hp count 
		}
	}
	//create the coin pickup using the pickup class playing a different sound when it gets picked up and incrementing the coin count
	if (m_pickup3->active()) {
		glm::mat4 coin_transform(1.0f);
		coin_transform = glm::translate(coin_transform, m_pickup3->position());
		coin_transform = glm::rotate(coin_transform, m_pickup3->rotation_amount(), glm::vec3(0.f, 5.f, 0.f));
		coin_transform = glm::scale(coin_transform, m_pickup3->scale());
		engine::renderer::submit(mesh_shader, coin_transform, m_pickup3);
		glm::vec3 d = m_pickup3->position() - m_player.object()->position();
		if (glm::length(d) < 1.f) {
			m_audio_manager->play_spatialised_sound("coinpick", m_3d_camera.position(), glm::vec3(m_pickup3->position().x, 0.f, m_pickup3->position().z));
			ammo_count += 10;
			m_pickup3->set_position(glm::vec3(0.f, -100.f, 0.f));//ammo pack, increase ammo on pickup
		}
	}
	if (m_pickup4->active()) {
		glm::mat4 coin_transform1(1.0f);
		coin_transform1 = glm::translate(coin_transform1, m_pickup4->position());
		coin_transform1 = glm::rotate(coin_transform1, m_pickup4->rotation_amount(), glm::vec3(0.f, 5.f, 0.f));
		coin_transform1= glm::scale(coin_transform1, m_pickup4->scale());
		engine::renderer::submit(mesh_shader, coin_transform1, m_pickup4);
		glm::vec3 d = m_pickup4->position() - m_player.object()->position();
		if (glm::length(d) < 1.f) {
			m_audio_manager->play_spatialised_sound("coinpick", m_3d_camera.position(), glm::vec3(m_pickup4->position().x, 0.f, m_pickup4->position().z));
			m_pickup4->set_position(glm::vec3(0.f, -100.f, 0.f));
			coin_count++;
		}
	}
	//adding the npcs into the game, name is differnt
	glm::mat4 z2_transform(1.0f);
	z2_transform = glm::translate(z2_transform, m_zombie2->position());
	z2_transform = glm::rotate(z2_transform, m_zombie2->rotation_amount(), glm::vec3(2.0f, 5.f, 0.f));
	z2_transform = glm::scale(z2_transform, m_zombie2->scale());
	engine::renderer::submit(mesh_shader, z2_transform, m_zombie2);

	//bot npc added, moves to always look at the camera in update method
	glm::mat4 bot_transform(1.0f);
	bot_transform = glm::translate(bot_transform, m_bot->position() - glm::vec3(m_bot-> offset().x, 0.f, m_bot->offset().z) * m_bot->scale().x);
	bot_transform = glm::rotate(bot_transform, m_bot->rotation_amount(), m_bot->rotation_axis());
	bot_transform = glm::scale(bot_transform, m_bot->scale());
	engine::renderer::submit(mesh_shader, bot_transform, m_bot);


	glm::mat4 zom_transform(1.0f);
	zom_transform = glm::translate(zom_transform, m_zombie->position() - glm::vec3(m_zombie-> offset().x, 0.f, m_zombie->offset().z) * m_zombie->scale().x);
	zom_transform = glm::rotate(zom_transform, m_zombie->rotation_amount(), m_zombie->rotation_axis());
	zom_transform = glm::scale(zom_transform, m_zombie->scale());
	engine::renderer::submit(mesh_shader, zom_transform, m_zombie);

	//the block of code is for all the buildings, could have been simplified
	glm::mat4 cube_transform(1.0f);
	cube_transform = glm::translate(cube_transform, m_cube->position());
	cube_transform = glm::rotate(cube_transform, m_cube->rotation_amount(),m_cube->rotation_axis());
	cube_transform = glm::scale(cube_transform,m_cube->scale());
	engine::renderer::submit(mesh_shader, cube_transform, m_cube);

	glm::mat4 cube_transform1(1.0f);
	cube_transform1 = glm::translate(cube_transform1, m_cube1->position());
	cube_transform1 = glm::rotate(cube_transform1, m_cube1->rotation_amount(),m_cube1->rotation_axis());
	cube_transform1 = glm::scale(cube_transform1,m_cube1->scale());
	engine::renderer::submit(mesh_shader, cube_transform1, m_cube1);

	glm::mat4 cube_transform2(1.0f);
	cube_transform2 = glm::translate(cube_transform2, m_cube2->position());
	cube_transform2 = glm::rotate(cube_transform2, m_cube2->rotation_amount(),m_cube2->rotation_axis());
	cube_transform2 = glm::scale(cube_transform2,m_cube2->scale());
	engine::renderer::submit(mesh_shader, cube_transform2, m_cube2);

	glm::mat4 cube_transform3(1.0f);
	cube_transform3 = glm::translate(cube_transform3, m_cube3->position());
	cube_transform3 = glm::rotate(cube_transform3, m_cube3->rotation_amount(),m_cube3->rotation_axis());
	cube_transform3 = glm::scale(cube_transform3,m_cube3->scale());
	engine::renderer::submit(mesh_shader, cube_transform3, m_cube3);

	glm::mat4 cube_transform4(1.0f);
	cube_transform4 = glm::translate(cube_transform4, m_cube4->position());
	cube_transform4 = glm::rotate(cube_transform4, m_cube4->rotation_amount(), m_cube4->rotation_axis());
	cube_transform4 = glm::scale(cube_transform4, m_cube4->scale());
	engine::renderer::submit(mesh_shader, cube_transform4, m_cube4);

	glm::mat4 cube_transform6(1.0f);
	cube_transform6 = glm::translate(cube_transform6, m_cube6->position());
	cube_transform6 = glm::rotate(cube_transform6, m_cube6->rotation_amount(), m_cube6->rotation_axis());
	cube_transform6 = glm::scale(cube_transform6, m_cube6->scale());
	engine::renderer::submit(mesh_shader, cube_transform6, m_cube6);

	glm::mat4 cube_transform5(1.0f);
	cube_transform5 = glm::translate(cube_transform5, m_cube5->position());
	cube_transform5 = glm::rotate(cube_transform5, m_cube5->rotation_amount(), m_cube5->rotation_axis());
	cube_transform5 = glm::scale(cube_transform5, m_cube5->scale());
	engine::renderer::submit(mesh_shader, cube_transform5, m_cube5);

	glm::mat4 cube_transform7(1.0f);
	cube_transform7 = glm::translate(cube_transform7, m_cube7->position());
	cube_transform7 = glm::rotate(cube_transform7, m_cube7->rotation_amount(), m_cube7->rotation_axis());
	cube_transform7 = glm::scale(cube_transform7, m_cube7->scale());
	engine::renderer::submit(mesh_shader, cube_transform7, m_cube7);

	//spotlights were created and set on some lamps (2) any more would de render the terrain
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("gNumSpotLights", (int)num_spot_lights);
	m_spotLight.submit(mesh_shader, 0);
	m_spotLight1.submit(mesh_shader, 1);
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("lighting_on", false);

	//m_lightsource_material->submit(mesh_shader);

	m_melee_effect->on_render(mesh_shader);

	m_ballistic_material->submit(mesh_shader);
	m_ballistic.on_render(mesh_shader);

	m_ballistic_material->submit(mesh_shader);
	m_melee.on_render(mesh_shader);

	engine::renderer::submit(mesh_shader, m_light->meshes().at(0), glm::translate(glm::mat4(1.f), m_spotLight.Position));
	engine::renderer::submit(mesh_shader, m_light1->meshes().at(0), glm::translate(glm::mat4(1.f), glm::vec3(-7.f, 2.45f, 25.f)));	
	std::dynamic_pointer_cast<engine::gl_shader>(mesh_shader)->set_uniform("lighting_on", true);

	//rendering the meshes for each effect
	engine::renderer::submit(mesh_shader, m_cube);
	m_mannequin_material->submit(mesh_shader);
	engine::renderer::submit(mesh_shader, m_player.object());
	engine::renderer::end_scene();

	engine::renderer::begin_scene(m_2d_camera, mesh_shader);
	m_playerhit->on_render(mesh_shader);
	engine::renderer::end_scene();

	engine::renderer::begin_scene(m_2d_camera, mesh_shader);
	m_playerhit2->on_render(mesh_shader);
	engine::renderer::end_scene();

	engine::renderer::begin_scene(m_2d_camera, mesh_shader);
	m_intro->on_render(mesh_shader);
	engine::renderer::end_scene();

	engine::renderer::begin_scene(m_3d_camera, mesh_shader);
	m_enemyhit->on_render(m_3d_camera, mesh_shader);
	engine::renderer::end_scene();



	// Render text add text and whatever
	const auto text_shader = engine::renderer::shaders_library()->get("text_2D");
	m_text_manager->render_text(text_shader, "Health: " + std::to_string(hp_count), 10.f, (float)engine::application::window().height() - 25.f, 0.5f, glm::vec4(1.f, 0.5f, 0.f, 1.f));
	m_text_manager->render_text(text_shader, "Coins: " + std::to_string(coin_count) , 10.f, (float)engine::application::window().height() - 50.f, 0.5f, glm::vec4(1.f, 0.5f, 0.f, 1.f));
	m_text_manager->render_text(text_shader, "Ammo: " + std::to_string(ammo_count) , 10.f, (float)engine::application::window().height() - 75.f, 0.5f, glm::vec4(1.f, 0.5f, 0.f, 1.f));
} 

void example_layer::on_event(engine::event& event) 
{
	//some key events are coded here
    if(event.event_type() == engine::event_type_e::key_pressed) 
    { 
        auto& e = dynamic_cast<engine::key_pressed_event&>(event); 
        if(e.key_code() == engine::key_codes::KEY_TAB) 
        { 
            engine::render_command::toggle_wireframe();
        }
		if (e.key_code() == engine::key_codes::KEY_Q)
		{
			if (ammo_count > 0) {
				m_ballistic.fire(m_3d_camera, glm::vec3(m_player.object()->position().x, m_player.object()->position().y + 1.f, m_player.object()->position().z), 10.0f);
				m_audio_manager->play_spatialised_sound("shot", m_3d_camera.position(), glm::vec3(m_player.object()->position().x, 0.f, m_player.object()->position().z));
				ammo_count--;
				//ammo count must be over 0 for the player to shoot
			}
		}
		if (e.key_code() == engine::key_codes::KEY_E)
		{
			if (melee_count > 0) {
				m_melee_effect->activate(0.3f, glm::vec3(m_player.object()->position().x, m_player.object()->position().y + 1.f, m_player.object()->position().z));
				m_melee.fire(m_3d_camera, glm::vec3(m_player.object()->position().x, m_player.object()->position().y + 1.f, m_player.object()->position().z), 1.0f);
			}//the knife must be picked up in order to use their melee
		}
		if (e.key_code() == engine::key_codes::KEY_P)
		{
			m_intro->activate();
		}
    } 
}
