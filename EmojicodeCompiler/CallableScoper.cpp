//
//  CallableScoper.cpp
//  Emojicode
//
//  Created by Theo Weidmann on 07/05/16.
//  Copyright © 2016 Theo Weidmann. All rights reserved.
//

#include <map>
#include "CallableScoper.hpp"
#include "CompilerErrorException.hpp"

Scope& CallableScoper::currentScope() {
    return scopes_.front();
}

void CallableScoper::popScopeAndRecommendFrozenVariables() {
    currentScope().recommendFrozenVariables();
    scopes_.pop_front();
}

Scope& CallableScoper::pushScope() {
    scopes_.push_front(Scope());
    return scopes_.front();
}

void CallableScoper::setVariable(const EmojicodeString &name, Variable value) {
    for (Scope &scope : scopes_) {
        if (scope.hasLocalVariable(name)) {
            scope.setLocalVariable(name, value);
            return;
        }
    }
    if (objectScope_ && objectScope_->hasLocalVariable(name)) {
        objectScope_->setLocalVariable(name, value);
        return;
    }
    scopes_.front().setLocalVariable(name, value);
}

std::pair<Variable&, bool> CallableScoper::getVariable(const EmojicodeString &name, SourcePosition errorPosition) {
    for (Scope &scope : scopes_) {
        if (scope.hasLocalVariable(name)) {
            return std::pair<Variable&, bool>(scope.getLocalVariable(name), false);
        }
    }
    if (objectScope_ && objectScope_->hasLocalVariable(name)) {
        return std::pair<Variable&, bool>(objectScope_->getLocalVariable(name), true);
    }
    throw CompilerErrorException(errorPosition, "Variable \"%s\" not defined.", name.utf8CString());
}

int CallableScoper::reserveVariableSlot() {
    return nextVariableID_++;
}

void CallableScoper::ensureNReservations(int n) {
    if (nextVariableID_ < n) {
        nextVariableID_ = n;
    }
}

std::pair<CallableScoper, int> CallableScoper::flattenedCopy(int argumentCount) const {
    int variableCount = 0;
    CallableScoper scoper = CallableScoper(objectScope());
    Scope &flattenedScope = scoper.pushScope();
    
    scoper.nextVariableID_ += argumentCount;
    
    for (auto &scope : scopes_) {
        for (auto &it : scope.map_) {
            auto variable = Variable(it.second.type, argumentCount + it.second.id,
                                     it.second.initialized, true, it.second.definitionToken);
            flattenedScope.setLocalVariable(it.first, variable);
        }
        scoper.nextVariableID_ += scope.map_.size();
        variableCount += scope.map_.size();
    }
    return std::pair<CallableScoper, int>(scoper, variableCount);
}