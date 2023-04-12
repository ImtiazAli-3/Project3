#include "pch.h"
#include "playerhit.h"
#include "quad.h"
//code for when the player is hit
//creates a red hue when the player is hit
//creates a green hue when health is gained
//processed in the example layer class
playerhit::playerhit(const std::string& path, float max_time, float width, float height)
{
	//set to false, so it is not active
	//when it needs to be active, it is called and the bool is set to true then back to false after it has been displayed
	m_max_time = max_time;
	m_texture = engine::texture_2d::create(path, true);

	m_transparency = 0.0f;

	m_quad = quad::create(glm::vec2(width, height));
	s_active = false;
}

playerhit::~playerhit()
{}

void playerhit::on_update(const engine::timestep& time_step)
{
	if (!s_active)
		return;

	m_timer += (float)time_step;

	m_transparency = 0.8f - 0.8f * m_timer / m_max_time; //how long it lasts for

	if (m_timer > m_max_time)
		s_active = false;
}

void playerhit::on_render(engine::ref<engine::shader> shader)
{
	if (!s_active)
		return;

	glm::mat4 transform(1.0f);
	transform = glm::translate(transform, glm::vec3(0.f, 0.f, 0.1f));

	std::dynamic_pointer_cast<engine::gl_shader>(shader)->set_uniform("transparency", m_transparency);
	std::dynamic_pointer_cast<engine::gl_shader>(shader)->set_uniform("has_texture", true);
	m_texture->bind();
	engine::renderer::submit(shader, m_quad->mesh(), transform);
	std::dynamic_pointer_cast<engine::gl_shader>(shader)->set_uniform("has_texture", false);
	std::dynamic_pointer_cast<engine::gl_shader>(shader)->set_uniform("transparency", 1.0f);


}

void playerhit::activate()
{
	s_active = true;
	m_transparency = 1.0f;
	m_timer = 0.0f;
}

engine::ref<playerhit> playerhit::create(const std::string& path, float max_time, float width, float height)
{
	return std::make_shared<playerhit>(path, max_time, width, height);
}
