/*
*    Copyright 2006 Andrew Wilkinson <aw@cs.york.ac.uk>.
*
*    This file is part of Minimal (http://www-users.cs.york.ac.uk/~aw/pylinda)
*
*    Minimal is free software; you can redistribute it and/or modify
*    it under the terms of the GNU Lesser General Public License as published by
*    the Free Software Foundation; either version 2.1 of the License, or
*    (at your option) any later version.
*
*    Minimal is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Lesser General Public License for more details.
*
*    You should have received a copy of the GNU Lesser General Public License
*    along with Minimal; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "minimal_internal.h"

MinimalLayer Minimal_defaultLayer;
static MinimalLayer Minimal_currentLayer;

void Minimal_Layer_init() {
    Minimal_defaultLayer = Minimal_createLayer();
    Minimal_defaultLayer->name = (char*)malloc(strlen("__default__")+1);
    strcpy(Minimal_defaultLayer->name, "__default__");
    Minimal_currentLayer = Minimal_defaultLayer;
}

void Minimal_Layer_finalise() {
    Minimal_delReference(Minimal_defaultLayer);
}

MinimalLayer Minimal_setCurrentLayer(MinimalLayer layer) {
    Minimal_addReference(layer);
    Minimal_delReference(Minimal_currentLayer);

    Minimal_currentLayer = layer;
    return layer;
}

MinimalLayer Minimal_getCurrentLayer() {
    Minimal_addReference(Minimal_currentLayer);

    return Minimal_currentLayer;
}

void Minimal_Layer_addTree(MinimalLayer layer, Minimal_SyntaxTree* tree) {
    switch(tree->type) {
    case ST_BLANK:
        break;
    case ST_SEQENTIAL_DEFS:
        Minimal_Layer_addTree(layer, tree->branch1);
        Minimal_Layer_addTree(layer, tree->branch2);
        break;
    case ST_TYPE_SPEC:
        {
        MinimalValue t = Minimal_typeSpec(tree->type_name, tree->type_def);
        Minimal_addName(&(layer->map), tree->type_name, t);
        }
        break;
    case ST_FUNCTION_DEF:
        {
        MinimalValue f;
        MinimalValue typespec = Minimal_getName(&(layer->map), tree->func_name);
        if(typespec != NULL && Minimal_isTypeSpec(typespec)) {
            Minimal_addReference(typespec->type_spec);
            tree->function->type_def = typespec->type_spec;
        }
        f = Minimal_function(tree->func_name, NULL, NULL, tree);
        Minimal_addName(&(layer->map), tree->func_name, f);
        }
        break;
    default:
        fprintf(stderr, "Unknown tree node type in Minimal_Layer_addTree (%i)\n", tree->type);
        return;
    }
}

MinimalLayer Minimal_createLayer() {
    MinimalLayer layer = Minimal_newReference(MINIMAL_LAYER, MinimalLayer, struct MinimalLayer_t);
    layer->name = NULL;
    layer->parent = NULL;
    Minimal_SyntaxMap_init(&(layer->map));
    return layer;
}

MinimalLayer Minimal_createLayer2(MinimalLayer parent) {
    MinimalLayer layer = Minimal_newReference(MINIMAL_LAYER, MinimalLayer, struct MinimalLayer_t);
    layer->name = NULL;
    Minimal_addReference(parent);
    layer->parent = parent;
    Minimal_SyntaxMap_init(&(layer->map));
    return layer;
}

void Minimal_Layer_free(MinimalLayer layer) {
    free(layer->name);
    Minimal_delReference(layer->parent);
    Minimal_SyntaxMap_empty(&(layer->map));
    free(layer);
}
