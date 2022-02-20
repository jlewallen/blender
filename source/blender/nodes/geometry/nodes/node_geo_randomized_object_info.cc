/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "BLI_math_matrix.h"

#include "DNA_collection_types.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "DNA_modifier_types.h"

#include "BKE_DerivedMesh.h"
#include "BKE_collection.h"
#include "BKE_idprop.h"
#include "BKE_object.h"

#include "DEG_depsgraph_build.h"
#include "DEG_depsgraph_query.h"

#include "NOD_derived_node_tree.hh"
#include "NOD_geometry.h"
#include "NOD_geometry_nodes_eval_log.hh"
#include "NOD_node_declaration.hh"

// #include "MOD_modifiertypes.h"
// #include "MOD_nodes.h"
// #include "MOD_nodes_evaluator.hh"
// #include "MOD_ui_common.h"

#include "FN_field.hh"
#include "FN_field_cpp_type.hh"
#include "FN_multi_function.hh"

#include "node_geometry_util.hh"

#include <algorithm>

bool evaluate_child_geometry(Depsgraph *depsgraph,
                             Scene *scene,
                             Object *ob,
                             NodesModifierData *nmd,
                             GeometrySet &input_geometry_set,
                             GeometrySet &output_geometry_set,
                             int seed);

namespace blender::nodes::node_geo_randomized_object_info_cc {

NODE_STORAGE_FUNCS(NodeGeometryRandomizedObjectInfo)

static void node_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::Object>(N_("Object")).hide_label();
  b.add_input<decl::Int>(N_("Seed"));
  b.add_output<decl::Geometry>(N_("Geometry"));
}

static void node_layout(uiLayout *layout, bContext *UNUSED(C), PointerRNA *ptr)
{
  uiItemR(layout, ptr, "transform_space", UI_ITEM_R_EXPAND, nullptr, ICON_NONE);
}

static void node_node_init(bNodeTree *UNUSED(tree), bNode *node)
{
  NodeGeometryRandomizedObjectInfo *data = MEM_cnew<NodeGeometryRandomizedObjectInfo>(__func__);
  data->transform_space = GEO_NODE_TRANSFORM_SPACE_ORIGINAL;
  node->storage = data;
}

static void node_geo_exec(GeoNodeExecParams params)
{
  const NodeGeometryRandomizedObjectInfo &storage = node_storage(params.node());
  const bool transform_space_relative = (storage.transform_space ==
                                         GEO_NODE_TRANSFORM_SPACE_RELATIVE);

  const Object *self_object = params.self_object();

  Object *object = params.get_input<Object *>("Object");
  if (object == nullptr) {
    params.set_default_remaining_outputs();
    return;
  }

  const float4x4 &object_matrix = object->obmat;
  const float4x4 transform = float4x4(self_object->imat) * object_matrix;

  if (params.output_is_required("Geometry")) {
    if (object == self_object) {
      params.error_message_add(NodeWarningType::Error,
                               TIP_("Geometry cannot be retrieved from the modifier object"));
      params.set_default_remaining_outputs();
      return;
    }

    const int seed = params.get_input<int>("Seed");
    printf("\nOK: seed=%d\n", seed);

    LISTBASE_FOREACH (ModifierData *, md, &object->modifiers) {
      if (md->type == eModifierType_Nodes) {
        NodesModifierData *nmd = (NodesModifierData *)md;
        if (nmd->node_group != nullptr) {
          /*
          int socket_index;
          LISTBASE_FOREACH_INDEX (bNodeSocket *, socket, &nmd->node_group->inputs, socket_index) {
            printf("NODE-GROUP: SOCKET: '%s' identifier='%s'\n", socket->name, socket->identifier);

            IDProperty *property = IDP_GetPropertyFromGroup(nmd->settings.properties,
                                                            socket->identifier);
            if (property != nullptr) {
              if (property->type == IDP_INT) {
                printf("NODE-GROUP: [%p] seed property %d\n", property, seed);
                IDP_Int(property) = seed;
              }
            }
          }
          */

          // NOTE I have no idea if this is the proper way to do this. Definitely
          // should also probably be the "original" geometry set/mesh data here?
          GeometrySet input_geometry_set = bke::object_get_evaluated_geometry_set(*object);
          GeometrySet output_geometry_set;

          Scene *scene = DEG_get_input_scene(params.depsgraph());
          if (!evaluate_child_geometry(params.depsgraph(),
                                       scene,
                                       object,
                                       nmd,
                                       input_geometry_set,
                                       output_geometry_set,
                                       seed)) {
            params.error_message_add(NodeWarningType::Error,
                                     TIP_("Child geometry failed to evaluate"));
            params.set_default_remaining_outputs();
            return;
          }

          printf("NODE-GROUP: SET OUTPUT GEO\n");
          params.set_output("Geometry", output_geometry_set);
        }
      }
    }

    /*
    GeometrySet geometry_set = bke::object_get_evaluated_geometry_set(*object);
    if (transform_space_relative) {
      transform_geometry_set(geometry_set, transform, *params.depsgraph());
    }
    */
  }
}

}  // namespace blender::nodes::node_geo_randomized_object_info_cc

void register_node_type_geo_randomized_object_info()
{
  namespace file_ns = blender::nodes::node_geo_randomized_object_info_cc;

  static bNodeType ntype;

  geo_node_type_base(
      &ntype, GEO_NODE_RANDOMIZED_OBJECT_INFO, "Randomized Object Info", NODE_CLASS_INPUT);
  ntype.declare = file_ns::node_declare;
  node_type_init(&ntype, file_ns::node_node_init);
  node_type_storage(&ntype,
                    "NodeGeometryRandomizedObjectInfo",
                    node_free_standard_storage,
                    node_copy_standard_storage);
  ntype.geometry_node_execute = file_ns::node_geo_exec;
  ntype.draw_buttons = file_ns::node_layout;
  nodeRegisterType(&ntype);
}
