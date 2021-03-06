/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#pragma once

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/MeshRenderer.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "graphics/Light.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "Resources.h"

namespace Viry3D
{
    class DemoLightmap : public Demo
    {
    public:
        struct CameraParam
        {
            Vector3 pos;
            Vector3 rot;
            float fov;
            float near_clip;
            float far_clip;
        };
        CameraParam m_camera_param = {
            Vector3(0, 5, -10),
            Vector3(30, 0, 0),
            45,
            0.3f,
            1000
        };

        Camera* m_camera = nullptr;
        Camera* m_ui_camera = nullptr;
        Label* m_label = nullptr;
        Ref<Light> m_light;

        void InitCamera()
        {
            m_camera = Display::Instance()->CreateCamera();
            m_camera->SetDepth(0);
            m_camera->SetLocalPosition(m_camera_param.pos);
            m_camera->SetLocalRotation(Quaternion::Euler(m_camera_param.rot));

            m_camera->SetFieldOfView(m_camera_param.fov);
            m_camera->SetNearClip(m_camera_param.near_clip);
            m_camera->SetFarClip(m_camera_param.far_clip);
        }

        void InitLight()
        {
            m_light = RefMake<Light>(LightType::Directional);
            m_light->SetLocalRotation(Quaternion::Euler(45, 60, 0));
            m_light->SetAmbientColor(Color(0, 0, 0, 1));
            m_light->SetColor(Color(1, 1, 1, 1));
            m_light->SetIntensity(0.1f);
        }

        void InitShader()
        {
            RenderState render_state;
            auto shader = RefMake<Shader>(
                "#define LIGHTMAP 1",
                Vector<String>({ "Diffuse.vs" }),
                "",
                "#define LIGHTMAP 1",
                Vector<String>({ "Diffuse.fs" }),
                "",
                render_state);
            Shader::AddCache("Diffuse", shader);
        }

        void InitScene()
        {
            auto lightmap = Resources::LoadLightmap("res/scene/lightmap/scene.lightmap");

            auto node = Resources::LoadNode("res/scene/lightmap/scene.go");
            for (int i = 0; i < node->GetChildCount(); ++i)
            {
                Ref<MeshRenderer> mesh_renderer = RefCast<MeshRenderer>(node->GetChild(i));
                if (mesh_renderer)
                {
                    m_camera->AddRenderer(mesh_renderer);

                    auto material = mesh_renderer->GetMaterial();
                    if (material)
                    {
                        if (!material->GetTexture("u_texture"))
                        {
                            material->SetTexture("u_texture", Texture::GetSharedWhiteTexture());
                        }
                        material->SetLightProperties(m_light);

                        int lightmap_index = mesh_renderer->GetLightmapIndex();
                        if (lightmap_index >= 0)
                        {
                            material->SetTexture("u_lightmap", lightmap);
                        }
                    }
                }
            }
        }

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();
            m_ui_camera->SetDepth(1);
            m_ui_camera->SetClearFlags(CameraClearFlags::Nothing);

            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitLight();
            this->InitShader();
            this->InitScene();
            this->InitUI();
        }

        virtual void Done()
        {
            if (m_ui_camera)
            {
                Display::Instance()->DestroyCamera(m_ui_camera);
                m_ui_camera = nullptr;
            }
            if (m_camera)
            {
                Display::Instance()->DestroyCamera(m_camera);
                m_camera = nullptr;
            }

            Shader::RemoveCache("Diffuse");
        }

        virtual void Update()
        {
            if (m_label)
            {
                m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
            }
        }
    };
}
