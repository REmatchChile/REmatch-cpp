#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#define private public
#undef private
#include "parsing/visitors/visitor_headers.hpp"

#include "parsing/logical_variable_set_automaton/logical_va.hpp"

namespace rematch {

class Parser {


  public:

  private:
    std::shared_ptr<VariableFactory> vfact_ptr;
    std::shared_ptr<FilterFactory> ffact_ptr;
    std::unique_ptr<LogicalVA> logical_va;

  public:
    Parser(const std::string& input);

  private:
    void create_logical_va(const std::string& input);

};

} // end namespace rematch

#endif
