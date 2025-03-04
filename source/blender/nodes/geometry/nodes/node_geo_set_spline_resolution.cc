/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "BKE_spline.hh"

#include "node_geometry_util.hh"

namespace blender::nodes::node_geo_set_spline_resolution_cc {

static void node_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::Geometry>(N_("Geometry")).supported_type(GEO_COMPONENT_TYPE_CURVE);
  b.add_input<decl::Bool>(N_("Selection")).default_value(true).hide_value().supports_field();
  b.add_input<decl::Int>(N_("Resolution")).min(1).default_value(12).supports_field();
  b.add_output<decl::Geometry>(N_("Geometry"));
}

static void set_resolution_in_component(GeometryComponent &component,
                                        const Field<bool> &selection_field,
                                        const Field<int> &resolution_field)
{
  GeometryComponentFieldContext field_context{component, ATTR_DOMAIN_CURVE};
  const int domain_size = component.attribute_domain_size(ATTR_DOMAIN_CURVE);
  if (domain_size == 0) {
    return;
  }

  OutputAttribute_Typed<int> resolutions = component.attribute_try_get_for_output_only<int>(
      "resolution", ATTR_DOMAIN_CURVE);

  fn::FieldEvaluator evaluator{field_context, domain_size};
  evaluator.set_selection(selection_field);
  evaluator.add_with_destination(resolution_field, resolutions.varray());
  evaluator.evaluate();

  resolutions.save();
}

static void node_geo_exec(GeoNodeExecParams params)
{
  GeometrySet geometry_set = params.extract_input<GeometrySet>("Geometry");
  Field<bool> selection_field = params.extract_input<Field<bool>>("Selection");
  Field<int> resolution_field = params.extract_input<Field<int>>("Resolution");

  bool only_poly = true;
  geometry_set.modify_geometry_sets([&](GeometrySet &geometry_set) {
    if (geometry_set.has_curve()) {
      if (only_poly) {
        for (const SplinePtr &spline : geometry_set.get_curve_for_read()->splines()) {
          if (ELEM(spline->type(), Spline::Type::Bezier, Spline::Type::NURBS)) {
            only_poly = false;
            break;
          }
        }
      }
      set_resolution_in_component(geometry_set.get_component_for_write<CurveComponent>(),
                                  selection_field,
                                  resolution_field);
    }
  });

  if (only_poly) {
    params.error_message_add(NodeWarningType::Warning,
                             TIP_("Input geometry does not contain a Bezier or NURB spline"));
  }
  params.set_output("Geometry", std::move(geometry_set));
}

}  // namespace blender::nodes::node_geo_set_spline_resolution_cc

void register_node_type_geo_set_spline_resolution()
{
  namespace file_ns = blender::nodes::node_geo_set_spline_resolution_cc;

  static bNodeType ntype;

  geo_node_type_base(
      &ntype, GEO_NODE_SET_SPLINE_RESOLUTION, "Set Spline Resolution", NODE_CLASS_GEOMETRY);
  ntype.geometry_node_execute = file_ns::node_geo_exec;
  ntype.declare = file_ns::node_declare;
  nodeRegisterType(&ntype);
}
