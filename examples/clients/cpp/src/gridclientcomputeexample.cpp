/* @cpp.file.header */

/*  _________        _____ __________________        _____
 *  __  ____/___________(_)______  /__  ____/______ ____(_)_______
 *  _  / __  __  ___/__  / _  __  / _  / __  _  __ `/__  / __  __ \
 *  / /_/ /  _  /    _  /  / /_/ /  / /_/ /  / /_/ / _  /  _  / / /
 *  \____/   /_/     /_/   \_,__/   \____/   \__,_/  /_/   /_/ /_/
 */

/**
 * <summary>
 * This example demonstrates use of GridGain C++ remote client API. To compile this example
 * you first need to compile the API, located in GRIDGAIN_HOME/modules/clients/cpp (see README
 * file for compilation instructions).
 * <p>
 * To execute this example you should start one or more instances of <c>ClientExampleNodeStartup</c>
 * Java class which will start up a GridGain node with proper configuration (you can compile
 * and run this class from your favourite IDE).
 * <p>
 * You can also start a stand-alone GridGain instance by passing the path
 * to configuration file to <c>ggstart.{sh|bat}</c> script, like so:
 * <c>ggstart.sh examples/config/example-cache.xml'</c>.
 * <p>
 * Note that this example requires <c>org.gridgain.examples.misc.client.api.ClientExampleTask</c>
 * class to be present in remote nodes' classpath. If remote nodes are run by <c>ggstart.{sh|bat}</c> script
 * then JAR file containing the examples code should be placed to <c>GRIDGAIN_HOME/libs</c> folder.
 * You can build <c>gridgain-examples.jar</c> by running <c>mvn package</c> in <c>GRIDGAIN_HOME/examples</c>
 * folder. After that <c>gridgain-examples.jar</c> will be generated by Maven in
 * <c>GRIDGAIN_HOME/examples/target</c> folder.
 * <p>
 * Before starting nodes you also need to enable <c>gridgain-rest-tcp</c> module. Simply move
 * <c>libs/optional/gridgain-rest-tcp<c> folder to <c>libs<c> folder so that the module is added to classpath.
 * <p>
 * After node has been started this example creates a client connection and performs some
 * Compute Grid related operations.
 * </summary>
 */

#include "gridclientapiexample.hpp"

using namespace std;

/**
 * Runs a Compute Grid client example.
 *
 * @param client A client reference.
 */
void clientComputeExample(TGridClientPtr& client) {
    TGridClientComputePtr clientCompute = client->compute();

    TGridClientNodeList nodes = clientCompute->nodes();

    if (nodes.empty()) {
        cerr << "Failed to connect to grid in compute example, make sure that it is started and connection "
                "properties are correct." << endl;

        GridClientFactory::stopAll();

        return;
    }

    cout << "Current grid topology: " << nodes.size() << endl;

    GridClientUuid randNodeId = nodes[0]->getNodeId();

    cout << "RandNodeId is " << randNodeId.uuid() << endl;

    TGridClientNodePtr p = clientCompute->node(randNodeId);

    TGridClientComputePtr prj = clientCompute->projection(*p);

    GridClientVariant rslt = prj->execute("org.gridgain.examples.misc.client.api.ClientExampleTask", "GridClientNode projection task arg");

    cout << ">>> GridClientNode projection: there are totally " << rslt.toString() <<
        " test entries on the grid" << endl;

    TGridClientNodeList prjNodes;

    prjNodes.push_back(p);

    prj = clientCompute->projection(prjNodes);

    rslt = prj->execute("org.gridgain.examples.misc.client.api.ClientExampleTask","Collection execution task arg");

    cout << ">>> Collection execution: there are totally " << rslt.toString() <<
        " test entries on the grid" << endl;

    std::function<bool (const GridClientNode&)> filter = [&randNodeId](const GridClientNode& node) { return node.getNodeId() == randNodeId; };
    prj = clientCompute->projection(filter);

    rslt = prj->execute("org.gridgain.examples.misc.client.api.ClientExampleTask", "Predicate execution task arg");

    cout << ">>> Predicate execution: there are totally " << rslt.toString() <<
        " test entries on the grid" << endl;

    // Balancing - may be random or round-robin. Users can create
    // custom load balancers as well.
    TGridClientLoadBalancerPtr balancer(new GridClientRandomBalancer());

    std::function<bool (const GridClientNode&)> filter1 = [&randNodeId](const GridClientNode& node) { return node.getNodeId() == randNodeId; };
    prj = clientCompute->projection(filter1, balancer);

    rslt = prj->execute("org.gridgain.examples.misc.client.api.ClientExampleTask", "Predicate execution with balancer task arg");

    cout << ">>> Predicate execution with balancer: there are totally " << rslt.toString() <<
            " test entries on the grid" << endl;

    // Now let's try round-robin load balancer.
    balancer = TGridClientLoadBalancerPtr(new GridClientRoundRobinBalancer());

    prj = prj->projection(prjNodes, balancer);

    rslt = prj->execute("org.gridgain.examples.misc.client.api.ClientExampleTask", "GridClientNode projection task arg");

    cout << ">>> GridClientNode projection: there are totally " << rslt.toString() <<
            " test entries on the grid" << endl;

    TGridClientFutureVariant futVal = prj->executeAsync("org.gridgain.examples.misc.client.api.ClientExampleTask", "Execute async task arg");

    cout << ">>> Execute async: there are totally " << futVal->get().toString() <<
       " test entries on the grid" << endl;

    vector<GridClientUuid> uuids;

    uuids.push_back(randNodeId);

    nodes = prj->nodes(uuids);

    cout << ">>> Nodes with UUID " << randNodeId.uuid() << ": ";

    for (size_t i = 0 ; i < nodes.size(); i++) {
        if (i != 0)
            cout << ", ";

        cout << *(nodes[i]);
    }

    cout << endl;

    // Nodes may also be filtered with predicate. Here
    // we create a projection containing only local node.
    std::function < bool(const GridClientNode&) > filter2 = [&randNodeId](const GridClientNode& node) { return node.getNodeId() == randNodeId; };
    nodes = prj->nodes(filter2);

    cout << ">>> Nodes filtered with predicate: ";

    for (size_t i = 0 ; i < nodes.size(); i++) {
        if (i != 0)
            cout << ", ";

        cout << *(nodes[i]);
    }

    cout << endl;

    // Information about nodes may be refreshed explicitly.
    TGridClientNodePtr clntNode = prj->refreshNode(randNodeId, true, true);

    cout << ">>> Refreshed node: " << *clntNode << endl;

    TGridClientNodeFuturePtr futClntNode = prj->refreshNodeAsync(randNodeId, false, false);

    cout << ">>> Refreshed node asynchronously: " << *futClntNode->get() << endl;

    // Nodes may also be refreshed by IP address.
    string clntAddr = "127.0.0.1";

    vector<GridClientSocketAddress> addrs = clntNode->availableAddresses(TCP);

    if (addrs.size() > 0)
        clntAddr = addrs[0].host();

    clntNode = prj->refreshNode(clntAddr, true, true);

    cout << ">>> Refreshed node by IP: " << *clntNode << endl;

    // Asynchronous version.
    futClntNode = prj->refreshNodeAsync(clntAddr, false, false);

    cout << ">>> Refreshed node by IP asynchronously: " << *futClntNode->get() << endl;

    // Topology as a whole may be refreshed, too.
    TGridClientNodeList top = prj->refreshTopology(true, true);

    cout << ">>> Refreshed topology: ";

    for (size_t i = 0 ; i < top.size(); i++) {
        if (i != 0)
            cout << ", ";

        cout << *top[i];
    }

    cout << endl;

    // Asynchronous version.
    TGridClientNodeFutureList topFut = prj->refreshTopologyAsync(false, false);

    cout << ">>> Refreshed topology asynchronously: ";

    top = topFut->get();

    for (size_t i = 0; i < top.size(); i++) {
        if (i != 0)
            cout << ", ";

        cout << *top[i];
    }

    cout << endl;

    cout << "End of example." << endl;
}

/**
 * Main method.
 *
 * @return Result code.
 */
int main () {
    try {
        GridClientConfiguration cfg = clientConfiguration();

        cout << "The client will try to connect to the following addresses:" << endl;

        vector<GridClientSocketAddress> srvrs = cfg.servers();

        for (vector<GridClientSocketAddress>::iterator i = srvrs.begin(); i < srvrs.end(); i++)
            cout << i->host() << ":" << i->port() << endl;

        TGridClientPtr client = GridClientFactory::start(cfg);

        clientComputeExample(client);
    }
    catch(exception& e) {
        cerr << "Caught unhandled exception: " << e.what() << endl;
    }

    GridClientFactory::stopAll();
}


