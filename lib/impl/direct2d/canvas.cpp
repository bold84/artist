/*=============================================================================
   Copyright (c) 2016-2020 Joel de Guzman

   Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
=============================================================================*/
#include <infra/support.hpp>
#include <artist/canvas.hpp>
#include <context.hpp>
#include <path_impl.hpp>
#include <variant>
#include <stack>

namespace cycfi::artist
{
   using namespace d2d;

   class canvas::canvas_state : public context_state
   {
   public:
                        canvas_state();
                        ~canvas_state();

      virtual void      update(render_target& cnv);
      virtual void      discard();

      artist::path&     path();

                        template <typename T>
      void              fill_paint(T const& info, render_target& target);

                        template <typename T>
      void              stroke_paint(T const& info, render_target& target);

      void              line_width(float w);
      void              fill(context& ctx, bool preserve);
      void              stroke(context& ctx, bool preserve);

      using line_cap_enum = canvas::line_cap_enum;
      using join_enum = canvas::join_enum;

      void              line_cap(line_cap_enum cap);
      void              line_join(join_enum join);
      void              miter_limit(float limit = 10);

      void              translate(point p);
      void              rotate(float rad);
      void              scale(point p);

   private:

      void              update_stroke_style();

      using paint_info = std::variant<
         color
       , canvas::linear_gradient
       , canvas::radial_gradient
      >;

      artist::path      _path;
      paint_info        _fill_info;
      brush*            _fill_paint = nullptr;
      paint_info        _stroke_info;
      brush*            _stroke_paint = nullptr;
      float             _line_width = 1;
      matrix2x2f        _matrix;

      d2d::stroke_style*_stroke_style = nullptr;
      line_cap_enum     _line_cap = line_cap_enum::butt;
      join_enum         _join = join_enum::miter_join;
      float             _miter_limit = 10;
   };

   canvas::canvas_state::canvas_state()
    : _matrix{matrix2x2f::Identity() }
   {
   }

   canvas::canvas_state::~canvas_state()
   {
      release(_fill_paint);
      release(_stroke_paint);
      release(_stroke_style);
   }

   void canvas::canvas_state::update(render_target& cnv)
   {
      auto make =
         [&cnv](auto const& info) -> brush*
         {
            return make_paint(info, cnv);
         };

      if (!_fill_paint)
         _fill_paint = std::visit(make, _fill_info);
      if (!_stroke_paint)
         _stroke_paint = std::visit(make, _stroke_info);
   }

   void canvas::canvas_state::discard()
   {
      release(_fill_paint);
      release(_stroke_paint);
   }

   artist::path& canvas::canvas_state::path()
   {
      return _path;
   }

   template <typename T>
   void canvas::canvas_state::fill_paint(T const& info, render_target& target)
   {
      if (!std::holds_alternative<T>(_fill_info)
         || info != std::get<T>(_fill_info))
      {
         _fill_info = info;
         release(_fill_paint);
         _fill_paint = make_paint(info, target);
      }
   }

   template <typename T>
   void canvas::canvas_state::stroke_paint(T const& info, render_target& target)
   {
      if (!std::holds_alternative<T>(_stroke_info)
         || info != std::get<T>(_stroke_info))
      {
         _stroke_info = info;
         release(_stroke_paint);
         _stroke_paint = make_paint(info, target);
      }
   }

   void canvas::canvas_state::line_width(float w)
   {
      _line_width = w;
   }

   void canvas::canvas_state::fill(context& ctx, bool preserve)
   {
      auto target = ctx.target();
      target->SetTransform(_matrix);
      _path.impl()->fill(*target, _fill_paint, preserve);
   }

   void canvas::canvas_state::stroke(context& ctx, bool preserve)
   {
      auto target = ctx.target();
      target->SetTransform(_matrix);
      _path.impl()->stroke(
         *target, _stroke_paint, _line_width, preserve, _stroke_style
      );
   }

   void canvas::canvas_state::line_cap(line_cap_enum cap)
   {
      if (_line_cap != cap)
      {
         _line_cap = cap;
         update_stroke_style();
      }
   }

   void canvas::canvas_state::line_join(join_enum join)
   {
      if (join != _join)
      {
         _join = join;
         update_stroke_style();
      }
   }

   void canvas::canvas_state::miter_limit(float limit)
   {
      if (_miter_limit != limit)
      {
         _miter_limit = limit;
         update_stroke_style();
      }
   }

   void canvas::canvas_state::update_stroke_style()
   {
      release(_stroke_style);
      _stroke_style = make_stroke_style(
         _line_cap, _join, _miter_limit
      );
   }

   void canvas::canvas_state::translate(point p)
   {
      _matrix = _matrix.Translation({ p.x, p.y }) * _matrix;
   }

   void canvas::canvas_state::rotate(float rad)
   {
      _matrix = _matrix.Rotation(rad * 180 / pi, { 0, 0 }) * _matrix;
   }

   void canvas::canvas_state::scale(point p)
   {
      _matrix = _matrix.Scale({ p.x, p.y }, { 0, 0 }) * _matrix;
   }

   canvas::canvas(canvas_impl_ptr context_)
    : _context{ context_ }
    , _state{ std::make_unique<canvas_state>() }
   {
      _context->state(_state.get());
   }

   canvas::~canvas()
   {
   }

   void canvas::pre_scale(point p)
   {
   }

   void canvas::translate(point p)
   {
      _state->translate(p);
   }

   void canvas::rotate(float rad)
   {
      _state->rotate(rad);
   }

   void canvas::scale(point p)
   {
      _state->scale(p);
   }

   void canvas::save()
   {
   }

   void canvas::restore()
   {
   }

   void canvas::begin_path()
   {
      _state->path().begin_path();
   }

   void canvas::close_path()
   {
      _state->path().close();
   }

   void canvas::fill()
   {
      _state->fill(*_context, false);
   }

   void canvas::fill_preserve()
   {
      _state->fill(*_context, true);
   }

   void canvas::stroke()
   {
      _state->stroke(*_context, false);
   }

   void canvas::stroke_preserve()
   {
      _state->stroke(*_context, true);
   }

   void canvas::clip()
   {
   }

   void canvas::move_to(point p)
   {
      _state->path().move_to(p);
   }

   void canvas::line_to(point p)
   {
     _state->path().line_to(p);
   }

   void canvas::arc_to(point p1, point p2, float radius)
   {
      _state->path().arc_to(p1, p2, radius);
   }

   void canvas::arc(
      point p, float radius,
      float start_angle, float end_angle,
      bool ccw
   )
   {
      _state->path().arc(p, radius, start_angle, end_angle, ccw);
   }

   void canvas::rect(struct rect r)
   {
      _state->path().add(r);
   }

   void canvas::round_rect(struct rect r, float radius)
   {
      _state->path().add(r, radius);
   }

   void canvas::circle(struct circle c)
   {
      _state->path().add(c);
   }

   void canvas::path(class path const& p)
   {
   }

   void canvas::quadratic_curve_to(point cp, point end)
   {
      _state->path().quadratic_curve_to(cp, end);
   }

   void canvas::bezier_curve_to(point cp1, point cp2, point end)
   {
      _state->path().bezier_curve_to(cp1, cp2, end);
   }

   void canvas::fill_style(color c)
   {
      _state->fill_paint(c, *_context->target());
   }

   void canvas::stroke_style(color c)
   {
      _state->stroke_paint(c, *_context->target());
   }

   void canvas::line_width(float w)
   {
      _state->line_width(w);
   }

   void canvas::line_cap(line_cap_enum cap_)
   {
      _state->line_cap(cap_);
   }

   void canvas::line_join(join_enum join_)
   {
      _state->line_join(join_);
   }

   void canvas::miter_limit(float limit)
   {
      _state->miter_limit(limit);
   }

   void canvas::shadow_style(point offset, float blur, color c)
   {
   }

   void canvas::global_composite_operation(composite_op_enum mode)
   {
   }

   void canvas::fill_style(linear_gradient const& gr)
   {
      _state->fill_paint(gr, *_context->target());
   }

   void canvas::fill_style(radial_gradient const& gr)
   {
      _state->fill_paint(gr, *_context->target());
   }

   void canvas::stroke_style(linear_gradient const& gr)
   {
      _state->stroke_paint(gr, *_context->target());
   }

   void canvas::stroke_style(radial_gradient const& gr)
   {
      _state->stroke_paint(gr, *_context->target());
   }

   void canvas::font(class font const& font_)
   {
   }

   void canvas::fill_text(std::string_view utf8, point p)
   {
   }

   void canvas::stroke_text(std::string_view utf8, point p)
   {
   }

   canvas::text_metrics canvas::measure_text(std::string_view utf8)
   {
      return {};
   }

   void canvas::text_align(int align)
   {
   }

   void canvas::text_align(text_halign align)
   {
   }

   void canvas::text_baseline(text_valign align)
   {
   }

   void canvas::draw(image const& pic, struct rect src, struct rect dest)
   {
   }

   void canvas::fill_rule(path::fill_rule_enum rule)
   {
      _state->path().fill_rule(rule);
   }
}
