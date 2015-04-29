#ifndef RAG_EDGE_H
#define RAG_EDGE_H 1

class RAGEdge
{
public:
    float weight;
    bool IsCaled;

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

// Some public method and overrides
public:
    RAGEdge& operator=(const RAGEdge& rhs);
    RAGEdge& operator+(const RAGEdge& rhs);
};

RAGEdge& RAGEdge::operator=(const RAGEdge& rhs)
{
    if(this == &rhs)
        return *this;

    this->weight = rhs.weight;
    return *this;
}

RAGEdge& RAGEdge::operator+(const RAGEdge& rhs)
{
    if(!rhs.IsCaled)
        return *this;
    this->weight = this->weight + rhs.weight;
    return *this;
}

#endif // RAG_EDGE_H
