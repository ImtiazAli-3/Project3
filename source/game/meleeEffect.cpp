#include "pch.h"
#include "meleeEffect.h"

meleeEffect::meleeEffect(glm::vec3 start_colour, glm::vec3 end_colour, float max_time)
{

	//creates a sphere where the player was at to simulate an attack
	//the attack is melee and changes colour as time goes on

	//size of sphere
	m_sphere = engine::sphere::create(15, 30, 0.1f);
	m_material = engine::material::create(0.0f, start_colour, glm::vec3(0.0f), glm::vec3(0.0f), 0.25f);

	m_start_colour = start_colour;
	m_end_colour = end_colour;

	m_max_time = max_time;
	s_active = false;
}

meleeEffect::~meleeEffect()
{}

void meleeEffect::on_update(const engine::timestep& time_step)
{
	if (!s_active)
		return;

	// Program the object to the get larger and change colour over time
	m_scale += (float)time_step;

	// Interpolate between start and end colours
	float f = m_timer / m_max_time;
	if (f > 1.0f)
		f = 1.0f;
	m_material->set_ambient((1 - f) * m_start_colour + f * m_end_colour);

	m_timer += (float)time_step;
	if (m_timer > m_max_time)
		s_active = false;
}

void meleeEffect::on_render(engine::ref<engine::shader> shader)
{
	if (!s_active)
		return;

	std::dynamic_pointer_cast<engine::gl_shader>(shader)->set_uniform("lighting_on", false);
	m_material->submit(shader);
	glm::mat4 transform(1.0f);
	transform = glm::translate(transform, m_position);
	transform = glm::scale(transform, glm::vec3(m_scale));

	engine::renderer::submit(shader, m_sphere->mesh(), transform);

	std::dynamic_pointer_cast<engine::gl_shader>(shader)->set_uniform("lighting_on", true);

}

void meleeEffect::activate(float radius, glm::vec3 position)
{
	s_active = true;
	m_position = position;
	m_timer = 0.0f;
	m_scale = radius;
}

engine::ref<meleeEffect> meleeEffect::create(glm::vec3 start_colour, glm::vec3 end_colour, float max_time)
{
	return std::make_shared<meleeEffect>(start_colour, end_colour, max_time);
}

