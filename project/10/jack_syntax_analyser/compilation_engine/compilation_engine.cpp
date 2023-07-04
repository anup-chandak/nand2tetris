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
#include "compilation_engine.h"

JackCompilationEngine::JackCompilationEngine()
    : tokeniser(NULL), out_stream(NULL), code_writer(NULL) {
    std::cerr
        << "The Jack Compilation Engine is not supposed to be instantiated"
           "without arguments !\n";
    exit(1);
}

JackCompilationEngine::JackCompilationEngine(std::string input_filename) {
    tokeniser = NULL;
    out_stream = NULL;
    code_writer = NULL;
    class_table.Clear();
    inner_table.Clear();
    inner_table.SetParent(class_table);
    unique_label = 0;
    std::string output_filename = input_filename;
    output_filename.replace(output_filename.end() - 4, output_filename.end(),
                            "vm");
    tokeniser = new JackTokeniser(input_filename.c_str());
    out_stream = new std::ofstream(output_filename.c_str());
    code_writer = new VmWriter(out_stream);
    std::cerr << "Compilation Engine instantiated for " << input_filename
              << "\n";
}

JackCompilationEngine::~JackCompilationEngine() {
    class_table.Clear();
    inner_table.Clear();
    delete tokeniser;
    if (out_stream != NULL && out_stream->is_open()) {
        out_stream->flush();
        out_stream->close();
    }
    delete out_stream;
    delete code_writer;
}

bool JackCompilationEngine::start() { return compileClass(); }

bool JackCompilationEngine::testAndEatIdent() {
    if (tokeniser->getTokenType() != JackTokenType::IDENT) {
        return false;
    } else {
        tokeniser->advance();
        return true;
    }
}

bool JackCompilationEngine::testAndEatIdent(SymbolEntry& hash_entry) {
    if (tokeniser->getTokenType() != JackTokenType::IDENT) {
        return false;
    } else {
        std::get<0>(hash_entry) = tokeniser->getToken();
        tokeniser->advance();
        return true;
    }
}

bool JackCompilationEngine::testAndEatIdent(std::string& hash_key) {
    if (tokeniser->getTokenType() != JackTokenType::IDENT) {
        return false;
    } else {
        hash_key = tokeniser->getToken();
        tokeniser->advance();
        return true;
    }
}

bool JackCompilationEngine::testAndEatSymbol(char expected_char) {
    if (tokeniser->symbol() != expected_char) {
        return false;
    } else {
        tokeniser->advance();
        return true;
    }
}

bool JackCompilationEngine::testAndEatKeyword(
    std::vector<JackKeyword> expected_keywords) {
    auto result = std::find(expected_keywords.begin(), expected_keywords.end(),
                            tokeniser->keyWord());

    if (result == expected_keywords.end()) {
        return false;
    } else {
        tokeniser->advance();
        return true;
    }
}

bool JackCompilationEngine::testAndEatType() {
    /* We test first for the 3 valid keywords int char and boolean
     * and if it's not good, look for an identifier (class name)
     */
    std::vector<JackKeyword> valid_types = {
        JackKeyword::INT_, JackKeyword::CHAR_, JackKeyword::BOOLEAN_};
    if (testAndEatKeyword(valid_types)) {
        return true;
    } else {
        return testAndEatIdent();
    }
}

bool JackCompilationEngine::testAndEatType(SymbolEntry& hash_entry) {
    /* We test first for the 3 valid keywords int char and boolean
     * and if it's not good, look for an identifier (class name)
     */
    std::vector<JackKeyword> valid_types = {
        JackKeyword::INT_, JackKeyword::CHAR_, JackKeyword::BOOLEAN_};
    if (tokeniser->keyWord() == JackKeyword::INT_ ||
        tokeniser->keyWord() == JackKeyword::CHAR_ ||
        tokeniser->keyWord() == JackKeyword::BOOLEAN_) {
        std::get<0>(hash_entry) == tokeniser->getToken();
        tokeniser->advance();
        return true;
    } else {
        return testAndEatIdent(hash_entry);
    }
}

bool JackCompilationEngine::compileClass() {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::CLASS_) {
        return false;
    } else {
        class_table.Clear();
        tokeniser->advance();
        if (!testAndEatIdent(class_name)) {
            return false;
        }
        if (!testAndEatSymbol('{')) {
            return false;
        }
        while (compileClassVarDec()) {
            compileClassVarDec();
        }
        while (compileSubroutine()) {
            compileSubroutine();
        }
        if (!testAndEatSymbol('}')) {
            return false;
        }
        return true;
    }
}

bool JackCompilationEngine::compileClassVarDec() {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::STATIC_ &&
        tokeniser->keyWord() != JackKeyword::FIELD_) {
        return false;
    } else {
        SymbolEntry new_var;
        std::string new_var_key;
        switch (tokeniser->keyWord()) {
            case JackKeyword::STATIC_:
                std::get<1>(new_var) = JackVariableKind::STATIC;
                break;
            case JackKeyword::FIELD_:
                std::get<1>(new_var) = JackVariableKind::FIELD;
                break;
            default:
                break;
        }
        tokeniser->advance();
        if (!testAndEatType(new_var)) {
            return false;
        }
        if (!testAndEatIdent(new_var_key)) {
            return false;
        } else {
            class_table.Insert(new_var_key, new_var);
        }
        // Read the remaining varNames
        while (testAndEatSymbol(',')) {
            testAndEatIdent(new_var_key);
            class_table.Insert(new_var_key, new_var);
        }

        if (!testAndEatSymbol(';')) {
            return false;
        }
        return true;
    }
}

bool JackCompilationEngine::compileSubroutine() {
    // constructor;function;method
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::CONSTRUCTOR_ &&
        tokeniser->keyWord() != JackKeyword::FUNCTION_ &&
        tokeniser->keyWord() != JackKeyword::METHOD_) {
        return false;
    } else {
        inner_table.Clear();
        JackKeyword subroutine_type = tokeniser->keyWord();
        std::string subroutine_name;
        /* bool subroutine_is_void; */
        tokeniser->advance();
        // void or type
        if (tokeniser->keyWord() != JackKeyword::VOID_) {
            /* subroutine_is_void = false; */
            if (!testAndEatType()) {
                return false;
            }
        } else {
            /* subroutine_is_void = true; */
            tokeniser->advance();
        }
        // subroutineName
        if (!testAndEatIdent(subroutine_name)) {
            return false;
        }
        // Symbol and parameter list
        if (!testAndEatSymbol('(')) {
            return false;
        }
        compileParameterList(subroutine_type);
        if (!testAndEatSymbol(')')) {
            return false;
        }

        if (!testAndEatSymbol('{')) {
            return false;
        }

        int local_var_count = 0;
        while (compileVarDec(local_var_count)) {
            compileVarDec(local_var_count);
        }

        code_writer->Function(class_name + "." + subroutine_name,
                              local_var_count);
        if (subroutine_type == JackKeyword::CONSTRUCTOR_) {
            code_writer->Constructor(class_table.FieldCount());
        } else if (subroutine_type == JackKeyword::METHOD_) {
            code_writer->Method();
        }

        compileStatements();

        if (!testAndEatSymbol('}')) {
            return false;
        }

        return true;
    }
}

bool JackCompilationEngine::compileParameterList(JackKeyword subroutine_type) {
    SymbolEntry new_var;
    std::get<1>(new_var) = JackVariableKind::ARGUMENT;
    std::string new_var_key;
    if (subroutine_type == JackKeyword::METHOD_) {
        inner_table.Insert("this", class_name, JackVariableKind::ARGUMENT);
    }
    if (testAndEatType(new_var)) {
        if (!testAndEatIdent(new_var_key)) {
            return false;
        }
        inner_table.Insert(new_var_key, new_var);
    }

    // Read the remaining parameters
    // If there's a comma, we have to have a parameter
    while (testAndEatSymbol(',')) {
        if (!testAndEatType(new_var)) {
            return false;
        }
        if (!testAndEatIdent(new_var_key)) {
            return false;
        }
        inner_table.Insert(new_var_key, new_var);
    }

    return true;
}

bool JackCompilationEngine::compileVarDec(int& local) {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::VAR_) {
        return false;
    } else {
        SymbolEntry new_var;
        std::get<1>(new_var) = JackVariableKind::LOCAL;
        std::string new_var_key;

        tokeniser->advance();

        if (!testAndEatType(new_var)) {
            return false;
        }

        if (!testAndEatIdent(new_var_key)) {
            return false;
        }
        local++;
        inner_table.Insert(new_var_key, new_var);

        // Read the remaining varNames
        while (testAndEatSymbol(',')) {
            testAndEatIdent(new_var_key);
            inner_table.Insert(new_var_key, new_var);
            local++;
        }

        if (!testAndEatSymbol(';')) {
            return false;
        }

        return true;
    }
}

bool JackCompilationEngine::compileStatements() {
    bool foundStatement = false;
    do {
        foundStatement = false;
        if (compileLet()) {
            foundStatement = true;
            continue;
        } else if (compileDo()) {
            foundStatement = true;
            continue;
        } else if (compileWhile()) {
            foundStatement = true;
            continue;
        } else if (compileReturn()) {
            foundStatement = true;
            continue;
        } else if (compileIf()) {
            foundStatement = true;
            continue;
        }
    } while (foundStatement);

    return true;
}

bool JackCompilationEngine::compileDo() {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::DO_) {
        return false;
    } else {
        tokeniser->advance();

        std::string ident;
        int arg_count = 0;
        if (!testAndEatIdent(ident)) {
            return false;
        }

        std::string class_called;
        // Test if it's a class.subRoutine call
        if (testAndEatSymbol('.')) {
            // TODO: Code duplication with CompileTerm
            if (inner_table.GetTypeOf(ident) !=
                "") {  // Then it is a method call
                *out_stream << "push " << inner_table.GetVmOutput(ident)
                            << "\n";
                arg_count++;
                class_called = inner_table.GetTypeOf(ident);
            } else {  // Else it is a function call
                class_called = ident;
            }
            ident = class_called + ".";
            std::string sub_name;
            if (!testAndEatIdent(sub_name)) {
                return false;
            }
            ident += sub_name;
        } else if (inner_table.GetTypeOf(ident) == "") {
            // Test if it's (implicit_this).method call
            *out_stream << "push pointer 0\n";
            arg_count++;
            ident = class_name + "." + ident;
        }

        if (!testAndEatSymbol('(')) {
            return false;
        }

        if (!compileExpressionList(arg_count)) {
            return false;
        }

        if (!testAndEatSymbol(')')) {
            return false;
        }

        if (!testAndEatSymbol(';')) {
            return false;
        }

        code_writer->Do(ident, arg_count);

        return true;
    }
}

bool JackCompilationEngine::compileWhile() {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::WHILE_) {
        return false;
    } else {
        int first_label = unique_label++;
        int second_label = unique_label++;
        code_writer->Label(first_label);
        tokeniser->advance();

        if (!testAndEatSymbol('(')) {
            return false;
        }

        if (!compileExpression()) {
            return false;
        }

        if (!testAndEatSymbol(')')) {
            return false;
        }

        code_writer->IfFirstPart(second_label);

        if (!testAndEatSymbol('{')) {
            return false;
        }

        compileStatements();

        if (!testAndEatSymbol('}')) {
            return false;
        }

        code_writer->Goto(first_label);
        code_writer->Label(second_label);

        return true;
    }
}

bool JackCompilationEngine::compileLet() {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::LET_) {
        return false;
    } else {
        tokeniser->advance();

        std::string target;
        bool is_array = false;
        if (!testAndEatIdent(target)) {
            return false;
        }

        // Test if it's an Array call
        if (testAndEatSymbol('[')) {
            is_array = true;
            code_writer->Push(inner_table, target);
            if (!compileExpression()) {
                return false;
            }
            if (!testAndEatSymbol(']')) {
                return false;
            }
            code_writer->Add();
        }

        if (!testAndEatSymbol('=')) {
            return false;
        }

        if (!compileExpression()) {
            return false;
        }

        if (!testAndEatSymbol(';')) {
            return false;
        }

        if (!is_array) {
            code_writer->Let(inner_table, target);
        } else {
            code_writer->LetArray();
        }
        return true;
    }
}

bool JackCompilationEngine::compileIf() {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::IF_) {
        return false;
    } else {
        int first_label = unique_label++;
        int second_label = unique_label++;
        tokeniser->advance();

        if (!testAndEatSymbol('(')) {
            return false;
        }

        if (!compileExpression()) {
            return false;
        }

        code_writer->IfFirstPart(first_label);

        if (!testAndEatSymbol(')')) {
            return false;
        }

        if (!testAndEatSymbol('{')) {
            return false;
        }

        compileStatements();

        if (!testAndEatSymbol('}')) {
            return false;
        }

        code_writer->IfMidPart(second_label, first_label);

        // Test if there's an else bloc :
        if (testAndEatKeyword({JackKeyword::ELSE_})) {
            if (!testAndEatSymbol('{')) {
                return false;
            }

            compileStatements();

            if (!testAndEatSymbol('}')) {
                return false;
            }
        }
        code_writer->Label(second_label);

        return true;
    }
}

bool JackCompilationEngine::compileReturn() {
    // Simple test because we need to write xml tag before token
    if (tokeniser->keyWord() != JackKeyword::RETURN_) {
        return false;
    } else {
        tokeniser->advance();

        if (tokeniser->symbol() == ';') {
            code_writer->ReturnVoid();
            tokeniser->advance();
            return true;
        } else if (tokeniser->keyWord() == JackKeyword::THIS_) {
            code_writer->ReturnThis();
            tokeniser->advance();
            if (!testAndEatSymbol(';')) {
                return false;
            }
        }

        compileExpression();

        if (!testAndEatSymbol(';')) {
            return false;
        }

        code_writer->Return();

        return true;
    }
}

bool JackCompilationEngine::compileTerm() {
    if (tokeniser->symbol() == '-' || tokeniser->symbol() == '~') {
        char unary_op = tokeniser->symbol();
        tokeniser->advance();
        if (!compileTerm()) {
            return false;
        }
        code_writer->UnaryOp(unary_op);
        return true;
    } else {
        if (tokeniser->getTokenType() == JackTokenType::INT_CONST) {
            code_writer->IntConst(tokeniser->intVal());
            tokeniser->advance();
            return true;
        } else if (tokeniser->getTokenType() == JackTokenType::STRING_CONST) {
            code_writer->StringConst(tokeniser->stringVal());
            tokeniser->advance();
            return true;
        } else if (tokeniser->keyWord() == JackKeyword::TRUE_ ||
                   tokeniser->keyWord() == JackKeyword::FALSE_ ||
                   tokeniser->keyWord() == JackKeyword::NULL_ ||
                   tokeniser->keyWord() == JackKeyword::THIS_) {
            code_writer->KeywordConst(tokeniser->keyWord());
            tokeniser->advance();
            return true;
        } else if (tokeniser->getTokenType() == JackTokenType::IDENT) {
            /* This case should handle :
             *   - varName
             *   - varName[expr]
             *   - subroutineCall
             */
            std::string ident = tokeniser->getToken();
            tokeniser->advance();
            if (testAndEatSymbol('[')) {  // varName[expr]
                *out_stream << "push " << inner_table.GetVmOutput(ident)
                            << "\n";
                if (!compileExpression()) {
                    return false;
                }
                if (!testAndEatSymbol(']')) {
                    return false;
                }
                code_writer->Add();
                code_writer->ArrayAccess();
            } else if (testAndEatSymbol('.')) {  // subroutineCall with .
                int arg_count = 0;
                std::string class_called;
                if (inner_table.GetTypeOf(ident) !=
                    "") {  // Then it is a method call
                    *out_stream << "push " << inner_table.GetVmOutput(ident)
                                << "\n";
                    arg_count++;
                    class_called = inner_table.GetTypeOf(ident);
                } else {  // Else it is a function call
                    class_called = ident;
                }

                std::string method;
                if (!testAndEatIdent(method)) {
                    return false;
                }

                if (!testAndEatSymbol('(')) {
                    return false;
                }

                if (!compileExpressionList(arg_count)) {
                    return false;
                }

                if (!testAndEatSymbol(')')) {
                    return false;
                }
                code_writer->SubroutineCall(class_called + "." + method,
                                            arg_count);
            } else if (testAndEatSymbol('(')) {  // subroutineCall no .
                // This means the class is implied and depends on the file
                int arg_count = 0;
                if (!compileExpressionList(arg_count)) {
                    return false;
                }

                if (!testAndEatSymbol(')')) {
                    return false;
                }

                code_writer->SubroutineCall(class_name + "." + ident,
                                            arg_count);
            } else {
                /* Else we already ate the varName and nothing
                 * else to do
                 */
                *out_stream << "push " << inner_table.GetVmOutput(ident)
                            << "\n";
            }
            return true;
        } else if (tokeniser->symbol() == '(') {  // (expr)
            tokeniser->advance();
            if (!compileExpression()) {
                return false;
            }
            if (!testAndEatSymbol(')')) {
                std::cerr << "Term -> ( -> expr -> expecting )\n";
                tokeniser->showState();
                *out_stream << std::endl;
                exit(1);
            }
            return true;
        }
    }
    return false;
}

bool JackCompilationEngine::compileExpression() {
    if (!compileTerm()) {
        return false;
    }

    while (std::string("-*/&|<>=+").find(tokeniser->symbol()) !=
           std::string::npos) {
        char operation = tokeniser->symbol();
        tokeniser->advance();
        if (!compileTerm()) {
            std::cerr << "Term -> op -> expecting term\n";
            tokeniser->showState();
            *out_stream << std::endl;
            exit(1);
        }
        code_writer->Op(operation);
    }
    return true;
}

bool JackCompilationEngine::compileExpressionList(int& function_args) {
    if (tokeniser->symbol() == ')') {
        return true;
    }

    while (compileExpression()) {
        function_args++;
        if (!testAndEatSymbol(',')) {
            if (tokeniser->symbol() == ')') {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}
