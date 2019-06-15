#include "planner/expression_binder/index_binder.hpp"

using namespace duckdb;
using namespace std;

IndexBinder::IndexBinder(Binder &binder, ClientContext &context) : ExpressionBinder(binder, context) {
}

BindResult IndexBinder::BindExpression(ParsedExpression &expr, index_t depth, bool root_expression) {
	switch (expr.expression_class) {
	case ExpressionClass::AGGREGATE:
		return BindResult("aggregate functions are not allowed in index expressions");
	case ExpressionClass::WINDOW:
		return BindResult("window functions are not allowed in index expressions");
	case ExpressionClass::SUBQUERY:
		return BindResult("cannot use subquery in index expressions");
	default:
		return ExpressionBinder::BindExpression(expr, depth);
	}
}
