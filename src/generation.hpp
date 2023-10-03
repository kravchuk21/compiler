#pragma once

#include "parser.hpp"

class Generator {
public:
    inline explicit Generator(NodeExit root)
        : m_root(std::move(root))
    {
    }

    [[nodiscard]] std::string generate() const
    {
        std::stringstream output;
        output << ".global _main\n.align 2\n_main:\n";
        output << "    mov X0, " << m_root.expr.int_lit.value.value() << "\n";
        output << "    mov X16, #1\n" ;
        output << "    svc 0";
        return output.str();
    }

private:
    const NodeExit m_root;
}; 