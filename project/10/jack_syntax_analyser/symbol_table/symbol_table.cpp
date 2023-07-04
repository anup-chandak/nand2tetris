/* Copyright 2017 Gerry Agbobada
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "symbol_table.h"

int* JackVariableTable::VarCount(JackVariableKind var_kind) {
    switch (var_kind) {
        case JackVariableKind::FIELD:
            return &field_count;
        case JackVariableKind::STATIC:
            return &static_count;
        case JackVariableKind::ARGUMENT:
            return &argument_count;
        case JackVariableKind::LOCAL:
            return &local_count;
        default:
            std::cerr
                << "JackVariableTable::VarCount : bad kind of Variable \n";
            return NULL;
    }
}

void JackVariableTable::Clear() {
    var_map.clear();
    field_count = 0;
    static_count = 0;
    argument_count = 0;
    local_count = 0;
}

void JackVariableTable::SetParent(const JackVariableTable& parent_scope) {
    p_parent_scope = &parent_scope;
}

bool JackVariableTable::Insert(std::string var_key, std::string var_type,
                               JackVariableKind var_kind) {
    if (var_map.find(var_key) != var_map.end()) {
        std::cerr << "Variable " << var_key << " already exists !\n";
        return false;
    } else {
        SymbolEntry var_new_tuple(var_type, var_kind, *VarCount(var_kind));
        var_map.insert(std::make_pair(var_key, var_new_tuple));
        /* std::cerr << var_key << " " << var_type << " " */
        /*           << getTextForKind(var_kind) << " " << *VarCount(var_kind)
         */
        /*           << " added to the table\n"; */
        ++*VarCount(var_kind);
        return true;
    }
}

bool JackVariableTable::Insert(std::string var_name, SymbolEntry var_tuple) {
    return Insert(var_name, std::get<0>(var_tuple), std::get<1>(var_tuple));
}

std::string JackVariableTable::GetVmOutput(std::string var_key) const {
    if (var_map.find(var_key) == var_map.end()) {
        if (p_parent_scope == NULL) {
            std::cerr << "Variable " << var_key << " not found !\n";
            return "";
        } else {
            return p_parent_scope->GetVmOutput(var_key);
        }
    } else {
        std::ostringstream output;
        SymbolEntry var_tuple = var_map.at(var_key);
        output << getTextForKind(std::get<1>(var_tuple));
        output << " " << std::get<2>(var_tuple);
        /* std::cerr << var_key << " " << output.str() << "\n"; */
        return output.str();
    }
}

JackVariableKind JackVariableTable::GetKindOf(std::string var_key) const {
    auto it = var_map.find(var_key);
    if (it == var_map.end()) {
        if (p_parent_scope != NULL) {
            return p_parent_scope->GetKindOf(var_key);
        } else {
            return JackVariableKind::NONE;
        }
    } else {
        return std::get<1>(it->second);
    }
}

std::string JackVariableTable::GetTypeOf(std::string var_key) const {
    auto it = var_map.find(var_key);
    if (it == var_map.end()) {
        if (p_parent_scope != NULL) {
            return p_parent_scope->GetTypeOf(var_key);
        } else {
            return "";
        }
    } else {
        return std::get<0>(it->second);
    }
}

int JackVariableTable::GetIndexOf(std::string var_key) const {
    auto it = var_map.find(var_key);
    if (it == var_map.end()) {
        if (p_parent_scope != NULL) {
            return p_parent_scope->GetIndexOf(var_key);
        } else {
            return -1;
        }
    } else {
        return std::get<2>(it->second);
    }
}

bool JackVariableTable::IsSymbol(std::string var_key) const {
    auto it = var_map.find(var_key);
    if (it == var_map.end()) {
        if (p_parent_scope != NULL) {
            return p_parent_scope->IsSymbol(var_key);
        } else {
            return false;
        }
    } else {
        return true;
    }
}
