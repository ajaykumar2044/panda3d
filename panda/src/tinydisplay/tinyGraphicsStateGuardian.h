// Filename: tinyGraphicsStateGuardian.h
// Created by:  drose (24Apr08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef TINYGRAPHICSSTATEGUARDIAN_H
#define TINYGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "graphicsStateGuardian.h"
#include "colorBlendAttrib.h"
#include "simpleLru.h"
#include "zmath.h"
#include "zbuffer.h"

class TinyTextureContext;
struct GLContext;
struct GLVertex;
struct GLMaterial;
struct GLTexture;

////////////////////////////////////////////////////////////////////
//       Class : TinyGraphicsStateGuardian
// Description : An interface to the TinyGL software rendering code
//               within this module.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyGraphicsStateGuardian : public GraphicsStateGuardian {
public:
  TinyGraphicsStateGuardian(GraphicsPipe *pipe,
                            TinyGraphicsStateGuardian *share_with);

  virtual ~TinyGraphicsStateGuardian();

  virtual void reset();
  virtual void free_pointers();
  virtual void close_gsg();

  virtual bool depth_offset_decals();

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void clear(DrawableRegion *clearable);

  virtual void prepare_display_region(DisplayRegionPipelineReader *dr,
                                      Lens::StereoChannel stereo_channel);
  virtual CPT(TransformState) calc_projection_mat(const Lens *lens);
  virtual bool prepare_lens();

  virtual bool begin_frame(Thread *current_thread);
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame(Thread *current_thread);

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomMunger *munger,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     bool force);
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual bool draw_points(const GeomPrimitivePipelineReader *reader,
                           bool force);
  virtual void end_draw_primitives();

  virtual bool framebuffer_copy_to_texture
  (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram
  (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual bool update_texture(TextureContext *tc, bool force);
  virtual void release_texture(TextureContext *tc);

  virtual void do_issue_light();
  virtual void bind_light(PointLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id);

private:
  void do_issue_transform();
  void do_issue_render_mode();
  void do_issue_cull_face();
  void do_issue_rescale_normal();
  void do_issue_material();
  void do_issue_texture();
  void do_issue_scissor();

  void set_scissor(float left, float right, float bottom, float top);

  bool apply_texture(TextureContext *tc);
  bool upload_texture(TinyTextureContext *gtc);
  bool upload_simple_texture(TinyTextureContext *gtc);
  bool setup_gltex(GLTexture *gltex, int x_size, int y_size, int num_levels);
  int get_tex_shift(int orig_size);

  static void copy_lum_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level);
  static void copy_alpha_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level);
  static void copy_one_channel_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level, int channel);
  static void copy_la_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level);
  static void copy_rgb_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level);
  static void copy_rgba_image(ZTextureLevel *dest, int xsize, int ysize, Texture *tex, int level);

  void setup_material(GLMaterial *gl_material, const Material *material);
  void do_auto_rescale_normal();
  static void load_matrix(M4 *matrix, const TransformState *transform);
  static int get_color_blend_op(ColorBlendAttrib::Operand operand);
  static ZB_lookupTextureFunc get_tex_filter_func(Texture::FilterType filter);

public:
  // Filled in by the Tiny*GraphicsWindow at begin_frame().
  ZBuffer *_current_frame_buffer;

private:
  // Allocated by prepare_display_region when necessary for a zoomed
  // display region.
  ZBuffer *_aux_frame_buffer;

  GLContext *_c;

  enum ColorMaterialFlags {
    CMF_ambient   = 0x001,
    CMF_diffuse   = 0x002,
  };
  int _color_material_flags;
  int _texturing_state;
  int _texfilter_state;
  bool _texture_replace;
  bool _filled_flat;
  bool _auto_rescale_normal;

  CPT(TransformState) _scissor_mat;

  // Used during being_draw_primitives() .. end_draw_primitives().
  int _min_vertex;
  int _max_vertex;
  GLVertex *_vertices;
  int _vertices_size;

  static PStatCollector _vertices_immediate_pcollector;
  static PStatCollector _draw_transform_pcollector;
  static PStatCollector _pixel_count_white_untextured_pcollector;
  static PStatCollector _pixel_count_flat_untextured_pcollector;
  static PStatCollector _pixel_count_smooth_untextured_pcollector;
  static PStatCollector _pixel_count_white_textured_pcollector;
  static PStatCollector _pixel_count_flat_textured_pcollector;
  static PStatCollector _pixel_count_smooth_textured_pcollector;
  static PStatCollector _pixel_count_white_perspective_pcollector;
  static PStatCollector _pixel_count_flat_perspective_pcollector;
  static PStatCollector _pixel_count_smooth_perspective_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "TinyGraphicsStateGuardian",
                  GraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyGraphicsStateGuardian.I"

#endif
