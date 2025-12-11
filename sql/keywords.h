#pragma once

#include <stdlib.h>

#define KEYWORDS(X)                                                            \
  X("SELECT", "select")                                                        \
  X("FROM", "from")                                                            \
  X("UPDATE", "update")                                                        \
  X("DELETE", "delete")                                                        \
  X("WHERE", "where")                                                          \
  X("INNER JOIN", "inner join")                                                \
  X("JOIN", "join")                                                            \
  X("OUTER JOIN", "outer join")                                                \
  X("LEFT JOIN", "left join")                                                  \
  X("RIGHT JOIN", "right join")                                                \
  X("FULL JOIN", "full join")                                                  \
  X("CROSS JOIN", "cross join")                                                \
  X("INSERT", "insert")                                                        \
  X("INTO", "into")                                                            \
  X("VALUES", "values")                                                        \
  X("SET", "set")                                                              \
  X("CREATE", "create")                                                        \
  X("TABLE", "table")                                                          \
  X("ALTER", "alter")                                                          \
  X("DROP", "drop")                                                            \
  X("TRUNCATE", "truncate")                                                    \
  X("AND", "and")                                                              \
  X("OR", "or")                                                                \
  X("NOT", "not")                                                              \
  X("IN", "in")                                                                \
  X("BETWEEN", "between")                                                      \
  X("LIKE", "like")                                                            \
  X("IS", "is")                                                                \
  X("NULL", "null")                                                            \
  X("ORDER BY", "order by")                                                    \
  X("GROUP BY", "group by")                                                    \
  X("HAVING", "having")                                                        \
  X("DISTINCT", "distinct")                                                    \
  X("AS", "as")                                                                \
  X("LIMIT", "limit")                                                          \
  X("OFFSET", "offset")                                                        \
  X("UNION", "union")                                                          \
  X("UNION ALL", "union all")                                                  \
  X("INTERSECT", "intersect")                                                  \
  X("EXCEPT", "except")                                                        \
  X("CASE", "case")                                                            \
  X("WHEN", "when")                                                            \
  X("THEN", "then")                                                            \
  X("ELSE", "else")                                                            \
  X("END", "end")                                                              \
  X("EXISTS", "exists")                                                        \
  X("ANY", "any")                                                              \
  X("ALL", "all")                                                              \
  X("COUNT", "count")                                                          \
  X("SUM", "sum")                                                              \
  X("AVG", "avg")                                                              \
  X("MIN", "min")                                                              \
  X("MAX", "max")                                                              \
  X("ON", "on")                                                                \
  X("USING", "using")                                                          \
  X("PRIMARY KEY", "primary key")                                              \
  X("FOREIGN KEY", "foreign key")                                              \
  X("REFERENCES", "references")                                                \
  X("INDEX", "index")                                                          \
  X("VIEW", "view")                                                            \
  X("PROCEDURE", "procedure")                                                  \
  X("FUNCTION", "function")                                                    \
  X("TRIGGER", "trigger")                                                      \
  X("GRANT", "grant")                                                          \
  X("REVOKE", "revoke")                                                        \
  X("COMMIT", "commit")                                                        \
  X("ROLLBACK", "rollback")                                                    \
  X("BEGIN", "begin")                                                          \
  X("TRANSACTION", "transaction")                                              \
  X("DEFAULT", "default")                                                      \
  X("CHECK", "check")                                                          \
  X("UNIQUE", "unique")                                                        \
  X("NOT NULL", "not null")                                                    \
  X("AUTO_INCREMENT", "auto_increment")                                        \
  X("CASCADE", "cascade")                                                      \
  X("CONSTRAINT", "constraint")

#define UPPER(u, l) u,
#define LOWER(u, l) l,

static char *vocabulary[] = {KEYWORDS(UPPER) NULL};

static char *lower_vocabulary[] = {KEYWORDS(LOWER) NULL};
