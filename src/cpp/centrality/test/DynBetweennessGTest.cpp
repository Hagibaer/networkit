/*
 * DynBetweennessGTest.cpp
 *
 *  Created on: 05.08.2014
 *      Author: ebergamini, cls
 */

#include "DynBetweennessGTest.h"
#include "../Betweenness.h"
#include "../Betweenness2.h"
#include "../DynApproxBetweenness.h"
#include "../ApproxBetweenness.h"
#include "../ApproxBetweenness2.h"
#include "../DynBetweenness.h"
#include "../../io/METISGraphReader.h"
#include "../../auxiliary/Log.h"
#include "../../graph/Sampling.h"

namespace NetworKit {

TEST_F(DynBetweennessGTest, testDynBetweennessSmallGraph) {
/* Graph:
   0    3   6
	\  / \ /
	2    5
	/  \ / \
   1    4   7
*/
	int n = 8;
	Graph G(n);

	G.addEdge(0, 2);
	G.addEdge(1, 2);
	G.addEdge(2, 3);
	G.addEdge(2, 4);
	G.addEdge(3, 5);
	G.addEdge(4, 5);
	G.addEdge(5, 6);
	G.addEdge(5, 7);

	DynBetweenness dynbc = DynBetweenness(G, true);
	Betweenness bc = Betweenness(G);
	dynbc.run();
	bc.run();
	std::vector<double> dynbc_scores = dynbc.scores();
	std::vector<double> bc_scores = bc.scores();

	int i;
	const double tol = 1e-8;
	for(i=0; i<n; i++) {
		EXPECT_NEAR(dynbc_scores[i], bc_scores[i], tol) << "Scores are different";
	}

	// edge insertions
	GraphEvent ev(GraphEvent::EDGE_ADDITION, 0, 6, 1.0);
	G.addEdge(ev.u, ev.v);
	bc.run();
	dynbc.update(ev);

	dynbc_scores = dynbc.scores();
	bc_scores = bc.scores();
	for(i=0; i<n; i++) {
		EXPECT_NEAR(dynbc_scores[i], bc_scores[i], tol) << "Scores are different";
	}

}


TEST_F(DynBetweennessGTest, testWeightedDynBetweennessSmallGraph) {
/* Graph:
0    3   6
	\  / \ /
	2    5
	/  \ / \
1    4   7
*/
	int n = 8;
	Graph G(n, true);

	G.addEdge(0, 2);
	G.addEdge(1, 2);
	G.addEdge(2, 3);
	G.addEdge(2, 4);
	G.addEdge(3, 5);
	G.addEdge(4, 5);
	G.addEdge(5, 6);
	G.addEdge(5, 7);

	DynBetweenness dynbc = DynBetweenness(G, true);
//	Graph G1 = Graph(G, false, false);
	Betweenness2 bc = Betweenness2(G);
	dynbc.run();
	bc.run();
	std::vector<double> dynbc_scores = dynbc.scores();
	std::vector<double> bc_scores = bc.scores();

	int i;
	const double tol = 1e-8;
	for(i=0; i<n; i++) {
		EXPECT_NEAR(dynbc_scores[i], bc_scores[i], tol) << "Scores are different";
	}

	// edge insertions
	GraphEvent ev(GraphEvent::EDGE_ADDITION, 0, 6, 1.0);
	G.addEdge(ev.u, ev.v);
//	G1.addEdge(ev.u, ev.v);
	bc.run();
	dynbc.update(ev);
	std::cout<<"after"<<std::endl;
	dynbc_scores = dynbc.scores();
	bc_scores = bc.scores();
	for(i=0; i<n; i++) {
		EXPECT_NEAR(dynbc_scores[i], bc_scores[i], tol) << "Scores are different";
	}

}

TEST_F(DynBetweennessGTest, testDynApproxBetweennessSmallGraph) {
/* Graph:
0    3   6
	\  / \ /
	2    5
	/  \ / \
1    4   7
*/
	int n = 8;
	Graph G(n);

	G.addEdge(0, 2);
	G.addEdge(1, 2);
	G.addEdge(2, 3);
	G.addEdge(2, 4);
	G.addEdge(3, 5);
	G.addEdge(4, 5);
	G.addEdge(5, 6);
	G.addEdge(5, 7);

	double epsilon = 0.01; // error
	double delta = 0.1; // confidence
	DynApproxBetweenness dynbc = DynApproxBetweenness(G, epsilon, delta);
	Betweenness bc = Betweenness(G);
	dynbc.run();
	bc.run();
	std::vector<double> dynbc_scores = dynbc.scores();
	std::vector<double> bc_scores = bc.scores();
	for(int i=0; i<n; i++) {
		std::cout<<dynbc_scores[i]-bc_scores[i]/double(n*(n-1))<<std::endl;
	}
	std::vector<GraphEvent> batch;
	batch.push_back(GraphEvent(GraphEvent::EDGE_ADDITION, 0, 6, 1.0));
	G.addEdge(batch[0].u, batch[0].v);
	bc.run();
	dynbc.update(batch);
	dynbc_scores = dynbc.scores();
	bc_scores = bc.scores();
	for(int i=0; i<n; i++) {
		std::cout<<dynbc_scores[i]-bc_scores[i]/double(n*(n-1))<<std::endl;
	}

}


TEST_F(DynBetweennessGTest, testDynVsStatic) {
	METISGraphReader reader;
	Graph G = reader.read("input/PGPgiantcompo.graph");
	count n = G.upperNodeIdBound();
	std::cout<<n<<std::endl;

	double epsilon = 0.1; // error
	double delta = 0.1; // confidence
	DynApproxBetweenness dynbc = DynApproxBetweenness(G, epsilon, delta, false);
	ApproxBetweenness bc = ApproxBetweenness(G, epsilon, delta);
	dynbc.run();
	bc.run();
	std::vector<double> dynbc_scores = dynbc.scores();
	std::vector<double> bc_scores = bc.scores();
	double err1=0;
	for(count i=0; i<n; i++) {
		//std::cout<<dynbc_scores[i]-bc_scores[i]/double(n*(n-1))<<std::endl;
		double x = dynbc_scores[i]-bc_scores[i];
		if (x > err1)
			err1 = x;
	}
	std::cout<<"Before the edge insertion: "<<err1<<std::endl;
	std::vector<GraphEvent> batch;
	count nInsertions = 10, i = 0;
	while (i < nInsertions) {
		node v1 = Sampling::randomNode(G);
		node v2 = Sampling::randomNode(G);
		if (v1 != v2 && !G.hasEdge(v1, v2)) {
			G.addEdge(v1, v2);
			batch.push_back(GraphEvent(GraphEvent::EDGE_ADDITION, v1, v2, 1.0));
			i++;
		}
	}
	bc.run();
	dynbc.update(batch);
	dynbc_scores = dynbc.scores();
	bc_scores = bc.scores();
	err1 = 0;
	for(count i=0; i<n; i++) {
		//std::cout<<dynbc_scores[i]-bc_scores[i]/double(n*(n-1))<<std::endl;
		double x = dynbc_scores[i]-bc_scores[i];
		if (x > err1)
			err1 = x;
	}
	std::cout<<"After the edge insertion: "<<err1<<std::endl;

}

TEST_F(DynBetweennessGTest, timeDynApproxBetweenness) {
	METISGraphReader reader;
	Graph G = reader.read("input/PGPgiantcompo.graph");

	double epsilon = 0.1; // error
	double delta = 0.1; // confidence
	DynApproxBetweenness dynbc = DynApproxBetweenness(G, epsilon, delta, false);
	INFO("initial run");
	dynbc.run();
	INFO("update");
	std::vector<GraphEvent> batch;
	count nInsertions = 100000, i = 0;
	while (i < nInsertions) {
		node v1 = Sampling::randomNode(G);
		node v2 = Sampling::randomNode(G);
		if (v1 != v2 && !G.hasEdge(v1, v2)) {
			G.addEdge(v1, v2);
			batch.push_back(GraphEvent(GraphEvent::EDGE_ADDITION, v1, v2, 1.0));
			i++;
		}
	}
	dynbc.update(batch);
}

TEST_F(DynBetweennessGTest, timeDynExactBetweenness) {
	METISGraphReader reader;
	Graph G = reader.read("input/PGPgiantcompo.graph");

	DynBetweenness dynbc = DynBetweenness(G);
	INFO("initial run");
	dynbc.run();
	INFO("update");
	GraphEvent ev;
	count nInsertions = 100, i = 0;
	while (i < nInsertions) {
		node v1 = Sampling::randomNode(G);
		node v2 = Sampling::randomNode(G);
		if (v1 != v2 && !G.hasEdge(v1, v2)) {
			G.addEdge(v1, v2);
			ev = GraphEvent(GraphEvent::EDGE_ADDITION, v1, v2, 1.0);
			dynbc.update(ev);
			i++;
		}
	}
}

TEST_F(DynBetweennessGTest, testCorrectnessDynExactBetweenness) {
	METISGraphReader reader;
	Graph G = reader.read("input/PGPgiantcompo.graph");
	int n = G.upperNodeIdBound();
	DynBetweenness dynbc = DynBetweenness(G);
	Betweenness bc = Betweenness(G);
	dynbc.run();
	bc.run();
	std::cout<<"Before the edge insertion: "<<std::endl;
	GraphEvent ev;
	count nInsertions = 10, i = 0;
	while (i < nInsertions) {
		node v1 = Sampling::randomNode(G);
		node v2 = Sampling::randomNode(G);
		if (v1 != v2 && !G.hasEdge(v1, v2)) {
			G.addEdge(v1, v2);
			ev = GraphEvent(GraphEvent::EDGE_ADDITION, v1, v2, 1.0);
			dynbc.update(ev);
			bc.run();
			std::vector<double> dynbc_scores = dynbc.scores();
			std::vector<double> bc_scores = bc.scores();
			int i;
			const double tol = 1e-6;
			for(i=0; i<n; i++) {
				EXPECT_NEAR(dynbc_scores[i], bc_scores[i], tol) << "Scores are different";
			}
			i++;
		}
	}
}

TEST_F(DynBetweennessGTest, testWeightedDynExactBetweenness) {
	METISGraphReader reader;
	Graph G1 = reader.read("input/PGPgiantcompo.graph");
	Graph G = Graph(G1, true, false);
	std::cout<<G.isWeighted()<<std::endl;
	int n = G.upperNodeIdBound();
	DynBetweenness dynbc = DynBetweenness(G);
	Betweenness bc = Betweenness(G);
	dynbc.run();
	bc.run();
	std::cout<<"Before the edge insertion: "<<std::endl;
	GraphEvent ev;
	count nInsertions = 10, i = 0;
	while (i < nInsertions) {
		node v1 = Sampling::randomNode(G);
		node v2 = Sampling::randomNode(G);
		if (v1 != v2 && !G.hasEdge(v1, v2)) {
			G.addEdge(v1, v2);
			ev = GraphEvent(GraphEvent::EDGE_ADDITION, v1, v2, 1.0);
			dynbc.update(ev);
			bc.run();
			std::vector<double> dynbc_scores = dynbc.scores();
			std::vector<double> bc_scores = bc.scores();
			int i;
			const double tol = 1e-6;
			for(i=0; i<n; i++) {
				EXPECT_NEAR(dynbc_scores[i], bc_scores[i], tol) << "Scores are different";
			}
			i++;
		}
	}
}

} /* namespace NetworKit */
