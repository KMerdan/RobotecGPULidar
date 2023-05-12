// Copyright 2023 Robotec.AI
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include "APIObject.hpp"
#include "math/Vector.hpp"
#include "rgl/api/core.h"
#include "TextureData.hpp"

#define INVALID_TEXTURE_ID -1

struct Texture : APIObject<Texture> {

public:

	~Texture();

	int GetID() const { return ID; }

	std::shared_ptr<TextureData> GetData() const { return data; }


	Vec2i GetResolution() const { return resolution; }


private:
	// TODO (prybicki) Should I pollute internal class with api enum?
	Texture( void* texels, rgl_texture_format format,  int resolution, int id);

	Texture(const Texture &) = delete; // non construction-copyable
	Texture &operator=(const Texture &) = delete; // non copyable

private:
	friend APIObject<Texture>;
	int ID;
	Vec2i resolution{-1};
	rgl_texture_format format;

	std::shared_ptr<TextureData> data;


};