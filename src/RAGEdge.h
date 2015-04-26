#ifndef RAG_EDGE_H
#define RAG_EDGE_H 1

class RAGEdge
{
private:
    float weight;

// Constructor and Destructor
public:
    RAGEdge() :
        weight(0) {};
    RAGEdge(float weight) :
        weight(weight) {};

// getter and setter
public:
    const float get_weight() const { return weight; };
    void set_weight(float weight) { this->weight = weight; };
};

#endif // RAG_EDGE_H
