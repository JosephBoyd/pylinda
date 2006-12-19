#    Copyright 2004-2006 Andrew Wilkinson <aw@cs.york.ac.uk>
#
#    This file is part of PyLinda (http://www-users.cs.york.ac.uk/~aw/pylinda)
#
#    PyLinda is free software; you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation; either version 2.1 of the License, or
#    (at your option) any later version.
#
#    PyLinda is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with PyLinda; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import _linda_server

builtin = ["bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64", "ieeesingle", "ieeedouble", "string"]

if _linda_server.register_types:
    from type_cache import lookupType

def identity(value):
    return value

def compare_unregistered(t1, t2, checked=None):
    try:
        assert t1.isType()
        assert t2.isType()
    except:
        print "Error matching types", t1, t2
        raise

    func = None

    print t1.type_id, t2.type_id
    if checked is None:
        checked = {(t1.type_id, t2.type_id): func}
    elif (t1.type_id != 0 and t2.type_id != 0) and (t1.type_id, t2.type_id) in checked:
        return lambda value: checked[(t1.type_id,t2.type_id)](value)
    elif (t1.type_id != 0 and t2.type_id != 0):
        checked[(t1.type_id, t2.type_id)] = func
    print checked

    try:
        if t1.isNil() and t2.isNil():
            func = identity
        elif t1.isId() and t2.isId():
            if t1.type_id != 0 or t2.type_id != 0:
                if t1.type_id == t2.type_id:
                    func = identity
                else:
                    func = compare(lookupType(t1.type_id), lookupType(t2.type_id), checked)
            elif t1.id in builtin or t2.id in builtin:
                print t1.id, t2.id
                if t1.id == t2.id:
                    func = identity
                else:
                    func = None
            else:
                try:
                    func = compare(t1.typemap[t1.id], t2.typemap[t2.id], checked)
                except IndexError:
                    raise IndexError, "%s in %s, or %s in %s" % (t1.id, t1.typemap.keys(), t2.id, t2.typemap.keys())
        elif t1.isProductType() and t2.isProductType():
            if len(t1) != len(t2):
                return None
            for i in range(len(t1)):
                e1, e2 = t1[i], t2[i]
                if not compare(e1, e2, checked):
                    func = None
            def func(value):
                l = []
                for i in range(len(t1)):
                    l.append(checked[(t1[i],t2[i])](value[i]))
                return tuple(l)
        elif t1.isSumType() and t2.isSumType():
            if len(t1) != len(t2):
                func = None
            map = [None for _ in range(len(t1))]
            for i in range(len(t1)):
                for j in range(len(t2)):
                    print "sum_type", map, i, j, len(t1), len(t2)
                    if j in map:
                        print "continue"
                        continue
                    print "compare"
                    f = compare(t1[i], t2[j], checked)
                    if f is not None:
                        print "got func"
                        map[i] = j
                print "done j"
                if map[i] is None:
                    func = None
                    break
            print "done i"
            if None not in map:
                def func(value):
                    e1 = t1[value.sum_pos]
                    np = map[value.sum_pos]
                    e2 = t2[np]
                    v = checked[(e1.type_id, e2.type_id)](value)
                    v.sum_pos = np
                    return v
        elif t1.isPtrType() and t2.isPtrType():
            f = compare(t1.ptrtype, t2.ptrtype, checked)
            if f is None:
                func = None
            else:
                func = identity
        elif t1.isFunctionType() and t2.isFunctionType():
            arg_func = compare(t1.arg, t2.arg, checked)
            res_func = compare(t1.result, t2.result, checked)
            raise SystemError
        else:
            print "different types", t1, t2
            print t1.isNil(), t1.isId(), t1.isProductType(), t1.isSumType(), t1.isPtrType(), t1.isFunctionType()
            print t2.isNil(), t2.isId(), t2.isProductType(), t2.isSumType(), t2.isPtrType(), t2.isFunctionType()
            func = None
    except:
        raise
    else:
        checked[(t1.type_id, t2.type_id)] = func
        return func

def compare_registered(t1, t2, checked=None):
    assert t1.isType()
    assert t2.isType()

    if checked is None:
        checked = [(t1, t2)]
    elif (t1, t2) in checked:
        return True
    else:
        checked.append((t1, t2))

    if t1.type_id == t2.type_id:
        return True

    try:
        if t1.isNil() and t2.isNil():
            return True
        elif t1.isId() and t2.isId():
            if t1.id in builtin or t2.id in builtin:
                return t1.id == t2.id
            else:
                return compare(t1.typemap[t1.id], t2.typemap[t2.id], checked)
        elif t1.isProductType() and t2.isProductType():
            if len(t1) != len(t2):
                return False
            for i in range(len(t1)):
                e1, e2 = t1[i], t2[i]
                if not compare(e1, e2, checked):
                    return False
            return True
        elif t1.isSumType() and t2.isSumType():
            if len(t1) != len(t2):
                return False
            for i in range(len(t1)):
                e1, e2 = t1[i], t2[i]
                if not compare(e1, e2, checked):
                    return False
            return True
        elif t1.isPtrType() and t2.isPtrType():
            return compare(t1.ptrtype, t2.ptrtype, checked)
        elif t1.isFunctionType() and t2.isFunctionType():
            return compare(t1.arg, t2.arg, checked) and compare(t1.result, t2.result, checked)
        else:
            return False
    finally:
        checked.pop()

#if _linda_server.register_types:
#    compare = compare_registered
#else:
compare = compare_unregistered
