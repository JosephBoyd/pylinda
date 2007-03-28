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
    Minimal_addReference(Minimal_defaultLayer);
    Minimal_currentLayer = Minimal_defaultLayer;
}

void Minimal_Layer_finalise() {
    Minimal_delReference(Minimal_defaultLayer); Minimal_defaultLayer = NULL;
    Minimal_delReference(Minimal_currentLayer); Minimal_currentLayer = NULL;
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

void Minimal_Layer_addTree(MinimalLayer layer, MinimalValue tree) {
    int i;
    if(tree->type != M_SYNTAX_TREE) {
        fprintf(stderr, "Value which is not a syntax tree passed to Minimal_Layer_addTree\n");
        return;
    }
    switch(tree->syntax_tree->type) {
    case ST_BLANK:
        break;
    case ST_SEQENTIAL_DEFS:
        for(i = 0; i < tree->syntax_tree->length; i++) {
            Minimal_Layer_addTree(layer, tree->syntax_tree->branches[i]);
        }
        break;
    case ST_TYPE_SPEC:
        {
        Minimal_addReference(tree);
        tree = Minimal_typeSpec(tree->syntax_tree->type_name, tree);
        Minimal_addName(&(layer->map), tree->type_name, tree);
        }
        break;
    case ST_FUNCTION_DEF:
        {
        MinimalValue f;
        MinimalValue typespec = Minimal_getName(layer, tree->syntax_tree->func_name);
        if(typespec != NULL && Minimal_isType(typespec)) {
            Minimal_addReference(typespec);
            tree->syntax_tree->type_spec = typespec;
        } else {
            tree->syntax_tree->type_spec = NULL;
        }
        if(tree->syntax_tree->parameter_list == NULL) { fprintf(stderr, "Error: Parameter list is NULL from tree.\n"); *((int*)NULL) = 1; }
        Minimal_addReference(tree->syntax_tree->parameter_list);
        Minimal_addReference(tree->syntax_tree->body);
        f = Minimal_function2(tree->syntax_tree->func_name, tree->syntax_tree->type_spec, tree->syntax_tree->parameter_list, tree->syntax_tree->body);
        Minimal_delReference(f->layer);
        Minimal_addReference(layer);
        f->layer = layer;
        if(f->parameter_list == NULL) { fprintf(stderr, "Error: Parameter list is NULL when creating function.\n"); *((int*)NULL) = 1; }
        if(typespec != NULL && Minimal_isType(typespec)) {
            Minimal_delReference(typespec);
        }
        Minimal_addName(&(layer->map), tree->syntax_tree->func_name, f);
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
    if(layer->parent != NULL) { Minimal_delReference(layer->parent); }
    Minimal_SyntaxMap_empty(&(layer->map));
    free(layer);
}

void Minimal_Layer_getReferences(struct CyclicGarbageList* list, MinimalLayer layer) {
    Minimal_SyntaxMap_getReferences(list, &(layer->map));

    if(layer->parent == NULL) { return; }

    Minimal_addToCyclicGarbageList(list, (MinimalValue)layer->parent);
}
