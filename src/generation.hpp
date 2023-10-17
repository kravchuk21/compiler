#pragma once

#include "parser.hpp"
#include <unordered_map>

class Generator {
public:
    inline explicit Generator(NodeProg prog)
        : m_prog(std::move(prog))
    {
    }

    void gen_expr(const NodeExpr& expr)
    {
        struct ExprVisitor {
            Generator* gen;
            void operator()(const NodeExprIntLit& expr_int_lit) const
            {
                gen->m_output << "    mov x0, " << expr_int_lit.int_lit.value.value() << "\n";
                gen->push("x0");
            }
            void operator()(const NodeExprIdent& expr_ident) const
            {
                if (!gen->m_vars.contains(expr_ident.ident.value.value())) {
                    std::cerr << "Undeclared identifier: " << expr_ident.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                const auto& var = gen->m_vars.at(expr_ident.ident.value.value());
                std::stringstream offset;
                std::cout << gen->m_stack_size << std::endl;
                std::cout << var.stack_loc << std::endl;
                // offset << (gen->m_stack_size - var.stack_loc - 2) * 16;
                offset << var.stack_loc * 16;
                // TODO: fix this
                gen->add(offset.str());
            }
        };

        ExprVisitor visitor { .gen = this };
        std::visit(visitor, expr.var);
    }

    void gen_stmt(const NodeStmt& stmt)
    {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit& stmt_exit) const
            {
                gen->gen_expr(stmt_exit.expr);
                gen->pop("x0");
                gen->m_output << "    mov x16, #1\n";
                gen->m_output << "    svc 0\n";
            }
            void operator()(const NodeStmtLet& stmt_let) const
            {
                if (gen->m_vars.contains(stmt_let.ident.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_let.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({ stmt_let.ident.value.value(), Var { .stack_loc = gen->m_stack_size } });
                gen->gen_expr(stmt_let.expr);
            }
        };

        StmtVisitor visitor { .gen = this };
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] std::string gen_prog()
    {
        m_output << ".global _main\n.align 2\n_main:\n";

        for (const NodeStmt& stmt : m_prog.stmts) {
            gen_stmt(stmt);
        }

        m_output << "    mov x0, 93\n";
        m_output << "    mov x16, #1\n";
        m_output << "    svc 0\n";
        return m_output.str();
    }

private:
    void push(const std::string& reg)
    {
        m_output << "    str " << reg << ", [sp, #" << m_stack_size * 16 << "]\n";
        m_stack_size++;
    }

    void pop(const std::string& reg)
    {
        m_output << "    ldr " << reg << ", [sp]\n";
        m_stack_size--;
    }
    void add(const std::string& sp)
    {
        m_output << "    add sp, sp, #" << sp << "\n";
    }

    struct Var {
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars {};
};