/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "UICanvasRenderer.h"
#include "GameObject.h"
#include "UISprite.h"
#include "UILabel.h"
#include "Debug.h"
#include "graphics/Material.h"
#include "graphics/Mesh.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(UICanvasRenderer);

	UICanvasRenderer::UICanvasRenderer():
		m_type(RenderType::BaseView),
		m_color(Color::White())
	{
	}

	UICanvasRenderer::~UICanvasRenderer()
	{
	}

	bool UICanvasRenderer::IsRoot() const
	{
		return !GetParentRect();
	}

	void UICanvasRenderer::SetColor(const Color& color)
	{
		m_color = color;

		auto mat = this->GetSharedMaterial();
		if (mat)
		{
			mat->SetMainColor(color);
		}
	}

	void UICanvasRenderer::DeepCopy(const Ref<Object>& source)
	{
		Renderer::DeepCopy(source);

		auto src = RefCast<UICanvasRenderer>(source);
		this->SetSortingOrder(src->GetSortingOrder());
		m_anchor_min = src->m_anchor_min;
		m_anchor_max = src->m_anchor_max;
		m_offset_min = src->m_offset_min;
		m_offset_max = src->m_offset_max;
		m_pivot = src->m_pivot;
		m_dirty = true;
	}

	void UICanvasRenderer::HandleUIEvent(const List<UICanvasRenderer*>& list)
	{
		UIEventHandler::HandleUIEvent(list);
	}

	void UICanvasRenderer::LateUpdate()
	{
		UpdateViews();
	}

	void UICanvasRenderer::OnTranformHierarchyChanged()
	{
		MarkDirty();
	}

	void UICanvasRenderer::MarkDirty()
	{
		m_dirty = true;
	}

	void UICanvasRenderer::FindViews()
	{
		m_views.Clear();
		List<Ref<Transform>> to_find;
		to_find.AddFirst(this->GetTransform());

		while (!to_find.Empty())
		{
			auto t = to_find.First();
			to_find.RemoveFirst();

			auto view = t->GetGameObject()->GetComponent<UIView>();
			if (view &&
				view->IsEnable() &&
				t->GetGameObject()->IsActiveSelf())
			{
				auto type_name = view->GetTypeName();
				if (type_name == "UISprite")
				{
					if (m_type == RenderType::BaseView || m_type == RenderType::Sprite)
					{
						m_type = RenderType::Sprite;
						m_views.Add(view);
					}
				}
				else if (type_name == "UILabel")
				{
					if (m_type == RenderType::BaseView || m_type == RenderType::Text)
					{
						m_type = RenderType::Text;
						m_views.Add(view);
					}
				}
				else if (type_name == "UIView")
				{
					m_views.Add(view);
				}
				else
				{
					assert(!"unknown view type");
				}
			}

			int child_count = t->GetChildCount();
			for (int i = child_count - 1; i >= 0; i--)
			{
				auto child = t->GetChild(i);
				auto canvas = child->GetGameObject()->GetComponent<UICanvasRenderer>();
				if (!canvas && child->GetGameObject()->IsActiveSelf())
				{
					to_find.AddFirst(child);
				}
			}
		}

		for (int i = 0; i < m_views.Size(); i++)
		{
			m_views[i]->SetRenderer(RefCast<UICanvasRenderer>(this->GetRef()));
		}
	}

	void UICanvasRenderer::UpdateViews()
	{
		if (!m_dirty)
		{
			return;
		}

		m_dirty = false;

		FindViews();

		if (!m_views.Empty())
		{
			auto mat = this->GetSharedMaterial();
			if (!mat)
			{
				if (m_type == RenderType::BaseView || m_type == RenderType::Sprite)
				{
					mat = Material::Create("UI/Sprite");
					this->SetSharedMaterial(mat);
				}
				else if (m_type == RenderType::Text)
				{
					mat = Material::Create("UI/Text");
					this->SetSharedMaterial(mat);
				}

				mat->SetMainColor(m_color);
			}

			Vector<Vector3> vertices;
			Vector<Vector2> uv;
			Vector<Color> colors;
			Vector<unsigned short> indices;

			for (auto& i : m_views)
			{
				// skip render empty view, which just used by ui event
				if (i->GetTypeName() == "UIView")
				{
					continue;
				}

				i->SetRenderer(RefCast<UICanvasRenderer>(this->GetRef()));
				i->FillVertices(vertices, uv, colors, indices);
				i->FillMaterial(mat);
			}

			if (!vertices.Empty())
			{
				// make pixel perfect
				auto world_mat = this->GetTransform()->GetLocalToWorldMatrix();
				auto world_invert_mat = this->GetTransform()->GetWorldToLocalMatrix();
				for (int i = 0; i < vertices.Size(); i++)
				{
					auto world_pos = world_mat.MultiplyPoint3x4(vertices[i]);
					world_pos.x = floor(world_pos.x);
					world_pos.y = floor(world_pos.y);
					vertices[i] = world_invert_mat.MultiplyPoint3x4(world_pos);
				}

				if (!m_mesh)
				{
					m_mesh = Mesh::Create(true);
				}
				m_mesh->vertices = vertices;
				m_mesh->uv = uv;
				m_mesh->colors = colors;
				m_mesh->triangles = indices;
				m_mesh->Update();
			}
		}
	}

	const VertexBuffer* UICanvasRenderer::GetVertexBuffer() const
	{
		if (m_mesh)
		{
			return m_mesh->GetVertexBuffer().get();
		}

		return NULL;
	}

	const IndexBuffer* UICanvasRenderer::GetIndexBuffer() const
	{
		if (m_mesh)
		{
			return m_mesh->GetIndexBuffer().get();
		}

		return NULL;
	}

	void UICanvasRenderer::GetIndexRange(int material_index, int& start, int& count) const
	{
		if (m_mesh)
		{
			m_mesh->GetIndexRange(material_index, start, count);
		}
	}
}
