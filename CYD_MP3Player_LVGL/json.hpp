#include <ArduinoJson.h>
#include "tree.hpp"

class JsonTree {
private:
  static Node *find_name(Node *node, const char* path) {
    for (auto &n : node->children) {
      if (strcmp(n->name.c_str(), path) == 0) {
        return n;
      }
    }
    return NULL;
  }

public:
  static bool tree2json(Node *node, JsonDocument &doc) {
    for (auto &n : node->children) {
      const char *name = n->name.c_str();

      if (n->meta.type == TYPE_NODE) {
        JsonDocument d;
        if (tree2json(n, d)) {
          doc[name] = d;
        }
      }

      else if (n->meta.checked) {
        doc[doc.size()] = name;
      }
    }

    return doc.size() ? true : false;
  }

  static void select_leaf(JsonVariantConst doc, Node *node) {
    if (doc.is<JsonObjectConst>()) {
      for (JsonPairConst kv : doc.as<JsonObjectConst>()) {
        Node *found = find_name(node, kv.key().c_str());
        if (found) {
          select_leaf(kv.value(), found);
        }
      }
    }

    else if (doc.is<JsonArrayConst>()) {
      for (JsonVariantConst value : doc.as<JsonArrayConst>()) {
        select_leaf(value, node);
      }
    }

    else if (!doc.isNull()) /* JsonVariantConst */ {
      Node *found = find_name(node, doc.as<const char*>());
      if (found) {
        found->meta.checked = LEAF_SELECTED;
      }
    }
  }
};