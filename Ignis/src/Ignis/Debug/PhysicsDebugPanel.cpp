#include "PhysicsDebugPanel.h"
#include <imgui.h>
#include "Ignis/Core/Log.h"

namespace ignis {

	void PhysicsDebugPanel::OnImGuiRender(PhysicsWorld* world, bool& isOpen)
	{
		if (!isOpen || !world)
			return;

		ImGui::Begin("Physics Debug", &isOpen);

		ShowWorldInfo(world);
		ImGui::Separator();

		ImGui::Text("Physics Bodies (%zu)", world->GetBodyCount());
		ImGui::Checkbox("Show Details", &m_showDetails);

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

		const char* typeStr = body->IsStatic() ? "Static" : 
		                      body->IsDynamic() ? "Dynamic" : "Kinematic";
		
		bool isSelected = (m_selectedBody == index);
		if (ImGui::Selectable(std::string("Body " + std::to_string(index) + " [" + typeStr + "]").c_str(), isSelected))
		{
			m_selectedBody = isSelected ? -1 : index;
		}

		if (m_showDetails && isSelected)
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
		static float gravityY = -9.81f;
		if (ImGui::InputFloat("Gravity Y", &gravityY, 0.0f, 0.0f, "%.2f"))
		{
			world->SetGravity(glm::vec3(0.0f, gravityY, 0.0f));
		}

		if (ImGui::Button("Reset Gravity"))
		{
			gravityY = -9.81f;
			world->SetGravity(glm::vec3(0.0f, gravityY, 0.0f));
		}

		ImGui::SameLine();

		if (ImGui::Button("Zero Gravity"))
		{
			gravityY = 0.0f;
			world->SetGravity(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		// Selected body controls
		if (m_selectedBody >= 0 && m_selectedBody < static_cast<int>(world->GetBodyCount()))
		{
			ImGui::Separator();
			ImGui::Text("Selected Body Controls");

			auto body = world->GetBody(m_selectedBody);
			if (body && body->IsDynamic())
			{
				if (ImGui::Button("Apply Upward Force"))
				{
					body->ApplyCentralImpulse(glm::vec3(0.0f, 5.0f, 0.0f));
					Log::CoreInfo("Applied upward force to body {}", m_selectedBody);
				}
			}
		}
	}

}