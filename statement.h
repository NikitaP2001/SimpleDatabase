#pragma once

class stmt {

public:
        enum class type {
        // terminal
                type_error,
                keyword,
                identifier,
                literal,
                op,
                separator,
        // non terminal
                query,
                selection,
                deletion,
                insertion,
                creation,
                drop,
                select_query,
                col_names,
                table_name,
                expression,
                primary_expression,
                values,
                column_definitions,
                type
        };
        stmt() = default;
        stmt(const stmt&) = default;
        virtual type getType() = 0;
};