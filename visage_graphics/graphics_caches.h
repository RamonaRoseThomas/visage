/* Copyright Vital Audio, LLC
 *
 * visage is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * visage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with visage.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "embedded_file.h"
#include "graphics_utils.h"

namespace visage {
  struct ShaderCacheMap;
  struct ProgramCacheMap;
  struct UniformCacheMap;
  struct EmbeddedFile;

  class ShaderCache {
  public:
    static ShaderCache* instance() {
      static ShaderCache cache;
      return &cache;
    }

    static bgfx::ShaderHandle& shaderHandle(const EmbeddedFile& file) {
      return instance()->handle(file);
    }

    static void swapShader(const EmbeddedFile& file, const char* data, int size) {
      return instance()->swap(file, data, size);
    }

    static void restoreShader(const EmbeddedFile& file) { return instance()->restore(file); }

  private:
    ShaderCache();
    ~ShaderCache();

    bgfx::ShaderHandle& handle(const EmbeddedFile& file) const;
    void swap(const EmbeddedFile& file, const char* data, int size) const;
    void restore(const EmbeddedFile& file) const;

    std::unique_ptr<ShaderCacheMap> cache_;
  };

  class ProgramCache {
  public:
    struct ShaderPair {
      EmbeddedFile vertex;
      EmbeddedFile fragment;
    };

    static ProgramCache* instance() {
      static ProgramCache cache;
      return &cache;
    }

    static bgfx::ProgramHandle& programHandle(const EmbeddedFile& vertex, const EmbeddedFile& fragment) {
      return instance()->handle(vertex, fragment);
    }

    static void refreshProgram(const EmbeddedFile& vertex, const EmbeddedFile& fragment) {
      instance()->reload(vertex, fragment);
    }

    static void restoreProgram(const EmbeddedFile& vertex, const EmbeddedFile& fragment) {
      instance()->restore(vertex, fragment);
    }

    static std::vector<ShaderPair> programList() { return instance()->listPrograms(); }

  private:
    ProgramCache();
    ~ProgramCache();

    std::vector<ShaderPair> listPrograms() const;

    bgfx::ProgramHandle& handle(const EmbeddedFile& vertex, const EmbeddedFile& fragment) const;
    void reload(const EmbeddedFile& vertex, const EmbeddedFile& fragment) const;
    void restore(const EmbeddedFile& vertex, const EmbeddedFile& fragment) const;

    std::unique_ptr<ProgramCacheMap> cache_;
  };

  class UniformCache {
  public:
    enum Type {
      Sampler,
      Vec4,
      Mat3,
      Mat4,
    };

    static UniformCache* instance() {
      static UniformCache cache;
      return &cache;
    }

    static bgfx::UniformHandle& uniformHandle(const char* name, Type type = Vec4) {
      return instance()->handle(name, type);
    }

  private:
    UniformCache();
    ~UniformCache();

    bgfx::UniformHandle& handle(const char* name, Type type, int size = 1) const;

    std::unique_ptr<UniformCacheMap> cache_;
  };
}