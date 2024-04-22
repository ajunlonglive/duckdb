//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/optimizer/join_order/cardinality_estimator.hpp
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "duckdb/planner/column_binding_map.hpp"
#include "duckdb/optimizer/join_order/query_graph.hpp"
#include "duckdb/optimizer/join_order/query_graph_manager.hpp"

#include "duckdb/optimizer/join_order/relation_statistics_helper.hpp"

namespace duckdb {

struct DenomInfo {
	DenomInfo(JoinRelationSet &numerator_relations, double filter_strength, double denominator)
	    : numerator_relations(numerator_relations), filter_strength(filter_strength),
	      denominator(denominator) {
	}

	JoinRelationSet &numerator_relations;
	double filter_strength;
	double denominator;
};

struct RelationsToTDom {
	//! column binding sets that are equivalent in a join plan.
	//! if you have A.x = B.y and B.y = C.z, then one set is {A.x, B.y, C.z}.
	column_binding_set_t equivalent_relations;
	//!	the estimated total domains of the equivalent relations determined using HLL
	idx_t tdom_hll;
	//! the estimated total domains of each relation without using HLL
	idx_t tdom_no_hll;
	bool has_tdom_hll;
	vector<FilterInfo *> filters;
	vector<string> column_names;

	explicit RelationsToTDom(const column_binding_set_t &column_binding_set)
	    : equivalent_relations(column_binding_set), tdom_hll(0), tdom_no_hll(NumericLimits<idx_t>::Maximum()),
	      has_tdom_hll(false) {};
};

class FilterInfoWithTotalDomains {
public:
	FilterInfoWithTotalDomains(FilterInfo *filter_info, RelationsToTDom &relation2tdom) : filter_info(filter_info),
	      tdom_hll(relation2tdom.tdom_hll), tdom_no_hll(relation2tdom.tdom_no_hll), has_tdom_hll(relation2tdom.has_tdom_hll) {
	}

	FilterInfo *filter_info;
	//!	the estimated total domains of the equivalent relations determined using HLL
	idx_t tdom_hll;
	//! the estimated total domains of each relation without using HLL
	idx_t tdom_no_hll;
	bool has_tdom_hll;
};

struct Subgraph2Denominator {
	optional_ptr<JoinRelationSet> relations;
	optional_ptr<JoinRelationSet> numerator_relations;
	double denom;
	double numerator_filter_strength;

	Subgraph2Denominator() : relations(nullptr), numerator_relations(nullptr), denom(1), numerator_filter_strength(1) {};
};

class CardinalityHelper {
public:
	CardinalityHelper() {
	}
	CardinalityHelper(double cardinality_before_filters, double filter_string)
	    : cardinality_before_filters(cardinality_before_filters), filter_strength(filter_string) {};

public:
	double cardinality_before_filters;
	double filter_strength;

	vector<string> table_names_joined;
	vector<string> column_names;
};

class CardinalityEstimator {
public:
	explicit CardinalityEstimator() {};

private:
	vector<RelationsToTDom> relations_to_tdoms;
	unordered_map<string, CardinalityHelper> relation_set_2_cardinality;
	JoinRelationSetManager set_manager;
	vector<RelationStats> relation_stats;

public:
	void RemoveEmptyTotalDomains();
	void UpdateTotalDomains(optional_ptr<JoinRelationSet> set, RelationStats &stats);
	void InitEquivalentRelations(const vector<unique_ptr<FilterInfo>> &filter_infos);

	void InitCardinalityEstimatorProps(optional_ptr<JoinRelationSet> set, RelationStats &stats);

	//! cost model needs estimated cardinalities to the fraction since the formula captures
	//! distinct count selectivities and multiplicities. Hence the template
	template <class T>
	T EstimateCardinalityWithSet(JoinRelationSet &new_set);

	//! used for debugging.
	void AddRelationNamesToTdoms(vector<RelationStats> &stats);
	void PrintRelationToTdomInfo();

private:
	double GetNumerator(JoinRelationSet &set);
	DenomInfo GetDenominator(JoinRelationSet &set);

	bool SingleColumnFilter(FilterInfo &filter_info);
	vector<idx_t> DetermineMatchingEquivalentSets(FilterInfo *filter_info);
	//! Given a filter, add the column bindings to the matching equivalent set at the index
	//! given in matching equivalent sets.
	//! If there are multiple equivalence sets, they are merged.
	void AddToEquivalenceSets(FilterInfo *filter_info, vector<idx_t> matching_equivalent_sets);

	double CalculateUpdatedDemo(Subgraph2Denominator left, Subgraph2Denominator right, FilterInfoWithTotalDomains &filter);
	JoinRelationSet &UpdateNumeratorRelations(Subgraph2Denominator left, Subgraph2Denominator right, FilterInfoWithTotalDomains &filter);

	void AddRelationTdom(FilterInfo &filter_info);
	bool EmptyFilter(FilterInfo &filter_info);
};

} // namespace duckdb
