#pragma once

#include "antlr4-runtime.h"
#include "parsing/bad_regex_exception.hpp"
#include "parsing/factories/filter_factory.hpp"
#include "parsing/factories/variable_factory.hpp"
#include "parsing/grammar/autogenerated/REmatchParserBaseVisitor.h"

namespace rematch {
namespace visitors {

class VariableFactoryVisitor : public REmatchParserBaseVisitor {
public:
  std::shared_ptr<VariableFactory> vfact_ptr;

  std::any visitRoot(REmatchParser::RootContext *ctx) override {
    std::any vfact = visit(ctx->alternation());
    VariableFactory& vfact_cast = std::any_cast<VariableFactory &>(vfact);
    vfact_ptr = std::make_shared<VariableFactory>(vfact_cast);

    return 0;
  }
private:
  std::any visitAlternation(REmatchParser::AlternationContext *ctx) override {
    std::any vfact = visit(ctx->expr().front());
    VariableFactory &vfact_cast = std::any_cast<VariableFactory &>(vfact);

    size_t children_size = ctx->expr().size();
    if (children_size > 1) {
      for (size_t i = 1; i < children_size; i++) {
        std::any rhs = visit(ctx->expr(i));
        VariableFactory &rhs_cast = std::any_cast<VariableFactory &>(rhs);
        vfact_cast.merge(rhs_cast);
      }
    }

    return vfact;
  }

  std::any visitExpr(REmatchParser::ExprContext *ctx) override {
    std::any vfact = visit(ctx->element().front());
    VariableFactory &vfact_cast = std::any_cast<VariableFactory &>(vfact);

    size_t children_size = ctx->element().size();
    if (children_size > 1) {
      for (size_t i = 1; i < children_size; i++) {
        std::any rhs = visit(ctx->element()[i]);
        VariableFactory &rhs_cast = std::any_cast<VariableFactory &>(rhs);
        vfact_cast.merge(rhs_cast);
      }
    }

    return vfact;
  }

  std::any visitElement(REmatchParser::ElementContext *ctx) override {
    return visit(ctx->group());
  }

  std::any visitGroup(REmatchParser::GroupContext *ctx) override {
    std::any vfact;

    if (ctx->parentheses()) {
      vfact = visit(ctx->parentheses());
    } else if (ctx->assignation()) {
      vfact = visit(ctx->assignation());
    } else {
      vfact = visit(ctx->atom());
    }

    return vfact;
  }

  std::any visitParentheses(REmatchParser::ParenthesesContext *ctx) override {
    return visit(ctx->alternation());
  }

  std::any visitAssignation(REmatchParser::AssignationContext *ctx) override {
    std::any vfact = visit(ctx->alternation());
    VariableFactory &vfact_cast = std::any_cast<VariableFactory &>(vfact);

    std::string var = ctx->varname()->getText();
    if (vfact_cast.contains(var)) {
      throw BadRegexException("Nested the same variables inside asignations");
    }
    vfact_cast.add(var);

    return vfact;
  }

  std::any visitAtom(REmatchParser::AtomContext*) override {
    return std::make_any<VariableFactory>();
  }
};
} // namespace visitors
} // namespace rematch
