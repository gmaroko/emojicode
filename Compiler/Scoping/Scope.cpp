//
//  Scope.cpp
//  Emojicode
//
//  Created by Theo Weidmann on 02/05/16.
//  Copyright © 2016 Theo Weidmann. All rights reserved.
//

#include "Scope.hpp"
#include "Compiler.hpp"
#include "CompilerError.hpp"
#include "Types/TypeDefinition.hpp"

namespace EmojicodeCompiler {

void Scope::setVariableInitialization(bool initd) {
    for (auto &it : map_) {
        if (initd) {
            it.second.initialize();
        }
        else {
            it.second.uninitialize();
        }
    }
}

void Scope::pushInitializationLevel() {
    for (auto &it : map_) {
        it.second.pushInitializationLevel();
    }
}

void Scope::popInitializationLevel() {
    for (auto &it : map_) {
        it.second.popInitializationLevel();
    }
}

Variable& Scope::declareVariable(const std::u32string &variable, const Type &type, bool constant,
                                 const SourcePosition &p) {
    return declareVariableWithId(variable, type, constant, VariableID(maxVariableId_++), p);
}

Variable& Scope::declareVariableWithId(const std::u32string &variable, const Type &type, bool constant, VariableID id,
                                       const SourcePosition &p) {
    if (hasLocalVariable(variable)) {
        throw CompilerError(p, "Cannot redeclare variable.");
    }
    Variable &v = map_.emplace(variable, Variable(type, id, constant, variable, p)).first->second;
    return v;
}

Variable& Scope::getLocalVariable(const std::u32string &variable) {
    return map_.find(variable)->second;
}

bool Scope::hasLocalVariable(const std::u32string &variable) const {
    return map_.count(variable) > 0;
}

void Scope::uninitializedVariablesCheck(const SourcePosition &p, const std::string &errorMessageFront,
                                        const std::string &errorMessageBack) {
    for (auto &it : map_) {
        Variable &cv = it.second;
        if (!cv.initialized() && cv.type().type() != TypeType::Optional && !cv.inherited()) {
            throw CompilerError(p, errorMessageFront, utf8(cv.name()), errorMessageBack);
        }
    }
}

void Scope::recommendFrozenVariables(Compiler *app) const {
    for (auto &it : map_) {
        const Variable &cv = it.second;
        if (!cv.constant() && !cv.mutated()) {
            app->warn(cv.position(), "Variable \"", utf8(cv.name()),
                      "\" was never mutated; consider making it a constant variable.");
        }
    }
}

}  // namespace EmojicodeCompiler
