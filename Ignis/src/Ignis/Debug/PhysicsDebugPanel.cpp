#include "PhysicsDebugPanel.h"
#include <imgui.h>
#include "Ignis/Core/Log.h"

namespace ignis {

	// EditorPanel interface implementation
	void PhysicsDebugPanel::OnImGuiRender()
	{
		OnImGuiRender(m_physics_world);
	}

	void PhysicsDebugPanel::OnImGuiRender(PhysicsWorld* world)
	{
		if (!world)
			return;

		ImGui::Begin("Physics Debug");

		ShowWorldInfo(world);
		ImGui::Separator();

		ImGui::Text("Physics Bodies (%zu)", world->GetBodyCount());
		ImGui::Checkbox("Show Details", &m_show_details);

		ImGui::BeginChild("BodiesList", ImVec2(0, 300), true);
		for (size_t i = 0; i < world->GetBodyCount(); i++)
		{
			auto body = world->GetBody(i);
			if (body)
				ShowBodyInfo(body.get(), static_cast<int>(i));
		}
		ImGui::EndChild();

		ImGui::Separator();
		ShowControls(world);

		ImGui::End();
	}

	void PhysicsDebugPanel::ShowBodyInfo(PhysicsBody* body, int index)
	{
		if (!body) return;

		const char* type_str = body->IsStatic() ? "Static" : 
		                      body->IsDynamic() ? "Dynamic" : "Kinematic";
		
		bool is_selected = (m_selected_body == index);
		if (ImGui::Selectable(std::string("Body " + std::to_string(index) + " [" + type_str + "]").c_str(), is_selected))
		{
			m_selected_body = is_selected ? -1 : index;
		}

		if (m_show_details && is_selected)
		{
			ImGui::Indent();
			glm::vec3 pos = body->GetPosition();
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

			if (body->IsDynamic())
			{
				glm::vec3 vel = body->GetLinearVelocity();
				ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);
				ImGui::Text("Speed: %.2f", glm::length(vel));
			}

			ImGui::Text("Mass: %.2f kg", body->GetMass());
			ImGui::Text("Friction: %.2f", body->GetFriction());
			ImGui::Text("Restitution: %.2f", body->GetRestitution());
			ImGui::Unindent();
		}
	}

    void PhysicsDebugPanel::ShowWorldInfo(PhysicsWorld* world)
	{
		ImGui::Text("World Information");
		ImGui::Separator();

		glm::vec3 gravity = world->GetGravity();
		ImGui::Text("Gravity: (%.2f, %.2f, %.2f)", gravity.x, gravity.y, gravity.z);
	}

    void PhysicsDebugPanel::ShowControls(PhysicsWorld* world)
	{
		ImGui::Text("Controls");
		ImGui::Separator();

		// Gravity input field (type-in instead of slider)
		static float gravity_y = -9.81f;
		if (ImGui::InputFloat("Gravity Y", &gravity_y, 0.0f, 0.0f, "%.2f"))
		{
			world->SetGravity(glm::vec3(0.0f, gravity_y, 0.0f));
		}

		if (ImGui::Button("Reset Gravity"))
		{
			gravity_y = -9.81f;
			world->SetGravity(glm::vec3(0.0f, gravity_y, 0.0f));
		}

		ImGui::SameLine();

		if (ImGui::Button("Zero Gravity"))
		{
			gravity_y = 0.0f;
			world->SetGravity(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		// Selected body controls
		if (m_selected_body >= 0 && m_selected_body < static_cast<int>(world->GetBodyCount()))
		{
			ImGui::Separator();
			ImGui::Text("Selected Body Controls");

			auto body = world->GetBody(m_selected_body);
			if (body && body->IsDynamic())
			{
				if (ImGui::Button("Apply Upward Force"))
				{
					body->ApplyCentralImpulse(glm::vec3(0.0f, 5.0f, 0.0f));
					Log::CoreInfo("Applied upward force to body {}", m_selected_body);
				}
			}
		}
	}

}