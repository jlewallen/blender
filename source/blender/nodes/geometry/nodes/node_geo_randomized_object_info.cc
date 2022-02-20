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
  const int seed = params.get_input<int>("Seed");

  printf("\nOK: seed=%d\n", seed);

  Object *object = params.get_input<Object *>("Object");
  if (object == nullptr) {
    params.set_default_remaining_outputs();
    return;
  }

  auto modified = false;

  LISTBASE_FOREACH (ModifierData *, md, &object->modifiers) {
    if (md->type == eModifierType_Nodes) {
      NodesModifierData *nmd = (NodesModifierData *)md;
      if (nmd->node_group != nullptr) {
        printf("NODE-GROUP: NODE-GROUP:\n");

        int socket_index;
        LISTBASE_FOREACH_INDEX (bNodeSocket *, socket, &nmd->node_group->inputs, socket_index) {
          printf("NODE-GROUP: SOCKET: '%s' identifier='%s'\n", socket->name, socket->identifier);

          IDProperty *property = IDP_GetPropertyFromGroup(nmd->settings.properties,
                                                          socket->identifier);
          if (property != nullptr) {
            if (property->type == IDP_INT) {
              printf("NODE-GROUP: [%p] seed property\n", property);
              IDP_Int(property) = seed;
              modified = true;
            }
          }
        }

        /*
        IDP_foreach_property(
            nmd->settings.properties,
            0,
            [](IDProperty *property, void *user_data) {
              ID *id = IDP_Id(property);
              char *repr = IDP_reprN(property, NULL);
              printf("IDProperty(%p): ", property);
              if (repr != NULL) {
                printf("%s\n", repr);
                MEM_freeN(repr);
              }
              else {
                printf("<null>\n");
              }

              if (property->type == IDP_FLOAT) {
                float value = IDP_Float(property);
                printf("Property-F: %f\n", value);
              }
              if (property->type == IDP_INT) {
                int value = IDP_Int(property);
                printf("Property-D: %d\n", value);
              }
            },
            nullptr);
            */

        /*
        // Unavailable from this level of the source tree, appears this is only available from MOD_
        layer.

        NodeTreeRefMap tree_refs;
        DerivedNodeTree tree{*nmd->node_group, tree_refs};

        blender::nodes::NodeMultiFunctions mf_by_node{tree};
        Map<DOutputSocket, GMutablePointer> group_inputs;
        Vector<DInputSocket> group_outputs;
        std::optional<geo_log::GeoLogger> geo_logger;

        blender::modifiers::geometry_nodes::GeometryNodesEvaluationParams eval_params;

        eval_params.input_values = group_inputs;
        eval_params.output_sockets = group_outputs;
        eval_params.mf_by_node = &mf_by_node;
        eval_params.modifier_ = nmd;
        eval_params.depsgraph = params.depsgraph();
        eval_params.self_object = object;
        eval_params.geo_logger = geo_logger.has_value() ? &*geo_logger : nullptr;
        blender::modifiers::geometry_nodes::evaluate_geometry_nodes(eval_params);
        GeometrySet output_geometry_set = std::move(
            *eval_params.r_output_values[0].get<GeometrySet>());
            */
      }
    }
  }

  if (modified) {
    Scene *scene = DEG_get_input_scene(params.depsgraph());
    printf("modified, building object data\n");

    // Crashes on memory free, likely from recurisve call into BKE_ layer
    // makeDerivedMesh(params.depsgraph(), scene, object, &CD_MASK_BAREMESH);

    printf("modified, building object data, done\n");
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

    GeometrySet geometry_set = bke::object_get_evaluated_geometry_set(*object);
    if (transform_space_relative) {
      transform_geometry_set(geometry_set, transform, *params.depsgraph());
    }

    params.set_output("Geometry", geometry_set);
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
