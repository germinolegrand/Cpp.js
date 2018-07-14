#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <numeric>

template<class T>
class ParseTree
{
private:
    std::vector<std::pair<int, T>> m_tree;

public:

    template<bool Const>
    struct NodeBase
    {
    public:
        using NodeValue = std::conditional_t<Const, T const, T>;

        using difference_type = size_t;
        using value_type = NodeValue;
        using pointer = NodeValue*;
        using reference = NodeValue&;
        using iterator_category = std::bidirectional_iterator_tag;

        NodeValue& operator*(){ return m_it->second; }
        NodeValue* operator->(){ return &m_it->second; }
        NodeBase& operator++();
        NodeBase& operator--();

        bool operator==(NodeBase const& other) const;
        bool operator!=(NodeBase const& other) const { return !operator==(other); }

        NodeBase begin() const;
        NodeBase end() const;

        bool is_root() const;
        NodeBase root() const;
        NodeBase parent() const;
        NodeBase previous_sibling() const;
        NodeBase next_sibling() const;
        size_t children() const;
        size_t size() const{ return children(); }
        size_t deep_size() const;
        bool empty() const;
        bool contains(NodeBase<true>) const;
        int depth() const;
        int child_depth(NodeBase const&) const;

        NodeBase append(T&& child);
        NodeBase append(T const& child);
        NodeBase append(NodeBase&& child);
        void clear_children();
        NodeBase remove();
        void skip_remove();
        void prune(NodeBase&);

        NodeBase(NodeBase<false> const& other): m_tree(other.m_tree), m_it(other.m_it) {}

    private:
        using Tree = std::conditional_t<Const,
            ParseTree const,
            ParseTree>;
        using TreeIterator = std::conditional_t<Const,
            typename decltype(ParseTree::m_tree)::const_iterator,
            typename decltype(ParseTree::m_tree)::iterator>;

        Tree* m_tree;
        TreeIterator m_it;

        friend class ParseTree;
        NodeBase(Tree& tree, TreeIterator it): m_tree(&tree), m_it(it) {}
        NodeBase(Tree* tree, TreeIterator it): m_tree(tree), m_it(it) {}

    public:
        int weight() const;
        int deep_weight() const;
    };

    using Node = typename ParseTree::template NodeBase<false>;
    using ConstNode = typename ParseTree::template NodeBase<true>;

    ParseTree() = default;
    ParseTree(T&&);
    ParseTree(Node&&);
    ParseTree(ConstNode const&);

    operator Node(){ return root(); }
    operator ConstNode(){ return root(); }

    Node root();
    ConstNode root() const;
    bool empty() const;
    size_t size() const;

    Node at();
    template<class...Args>
    Node at(size_t ind, Args...args);
    ConstNode at(size_t...) const;

    template<class U>
    friend std::ostream& operator<<(std::ostream&, ParseTree<U> const&);

    void append(Node& parent, T&& child);
    void append(Node& parent, T const& child);
    void clear_children(Node& parent);
    void remove(Node&);
    void skip_remove(Node&);
    void prune(Node&);
};

/// Constructors

template<class T>
ParseTree<T>::ParseTree(T&& root)
{
    m_tree.emplace_back(-1, std::forward<T>(root));
}

/// Observation

template<class T>
auto ParseTree<T>::root() -> Node
{
    return {*this, m_tree.begin()};
}

template<class T>
auto ParseTree<T>::root() const -> ConstNode
{
    return {*this, m_tree.begin()};
}

template<class T>
bool ParseTree<T>::empty() const
{
    return m_tree.empty();
}

template<class T>
size_t ParseTree<T>::size() const
{
    return m_tree.size();
}

template<class T>
auto ParseTree<T>::at() -> Node
{
    return root();
}

template<class T> template<class...Args>
auto ParseTree<T>::at(size_t ind, Args...args) -> Node
{
    return std::next(at(args...).begin(), ind);
}

template<class T>
std::ostream& operator<<(std::ostream& os, ParseTree<T> const& tree)
{
    int i = 0;
    for(auto& [lweight, value] : tree.m_tree){
        os << std::string(i, '>') << lweight << ':' << value << '\n';
        i += lweight;
    }
    return os;
}

/// Manipulation

template<class T>
void ParseTree<T>::append(Node& parent, T&& child)
{
    parent.append(std::forward<T>(child));
}

template<class T>
void ParseTree<T>::append(Node& parent, T const& child)
{
    parent.append(child);
}

template<class T>
void ParseTree<T>::clear_children(Node& parent)
{
    parent.clear_children();
}

template<class T>
void ParseTree<T>::remove(Node& node)
{
    node.remove();
}

template<class T>
void ParseTree<T>::skip_remove(Node& node)
{
    node.skip_remove();
}

template<class T>
void ParseTree<T>::prune(Node& node)
{
    root().prune(node);
}


////////////////////////////////////
/// ParseTree::NodeBase
////////////////////////////////////

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::operator++() -> NodeBase&
{
    for(auto i = 0; (i += (m_it++)->first) > 0;){

    }
    return *this;
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::operator--() -> NodeBase&
{
    for(auto i = 0; (i += (--m_it)->first) < 0;){

    }
    return *this;
}

template<class T> template<bool Const>
bool ParseTree<T>::NodeBase<Const>::operator==(NodeBase const& other) const
{
    return m_it == other.m_it;
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::begin() const -> NodeBase
{
    return {m_tree, std::next(m_it)};
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::end() const -> NodeBase
{
    NodeBase ret = *this;
    return ++ret;
}

template<class T> template<bool Const>
bool ParseTree<T>::NodeBase<Const>::is_root() const
{
    return m_tree->m_tree.begin() == m_it;
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::root() const -> NodeBase
{
    return {m_tree, m_tree->m_tree.begin()};
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::parent() const -> NodeBase
{
    auto it = m_it;
    for(auto i = 0; (i += (--it)->first) <= 0;){

    }
    return {m_tree, it};
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::previous_sibling() const -> NodeBase
{
    NodeBase previous{m_tree, m_it};
    --previous;
    return previous;
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::next_sibling() const -> NodeBase
{
    NodeBase next{m_tree, m_it};
    ++next;
    return next;
}

template<class T> template<bool Const>
size_t ParseTree<T>::NodeBase<Const>::children() const
{
    auto it = m_it;

    size_t count = 0;

    for(int i = 0; (i += (it++)->first) > 0;){
        if(i == 1){
            ++count;
        }
    }
    return count;
}

template<class T> template<bool Const>
size_t ParseTree<T>::NodeBase<Const>::deep_size() const
{
    return std::distance(begin().m_it, end().m_it);
}

template<class T> template<bool Const>
bool ParseTree<T>::NodeBase<Const>::empty() const
{
    return m_it->first < 1;
}

template<class T> template<bool Const>
bool ParseTree<T>::NodeBase<Const>::contains(NodeBase<true> other) const
{
    return m_it < other.m_it
        && other.m_it < std::next(m_it);
}

template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::depth() const
{
    return root().child_depth(*this);
}

/**
    @return > 0 if `child` is a descendent of `*this`
    @return 0 if `child` is `*this`
    [!maychange] @return -1 if `child` is not a descendent of `*this`
**/
template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::child_depth(NodeBase const& child) const
{
    int i = 0;
    auto it = m_it;
    for(i = it->first; it != child.m_it && i > 0; i += (++it)->first){

    }
    return i;
}



/// Manipulation

/**
    @return the node of the appended child
    @throw strong exception-garantee
**/
template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::append(T&& child) -> NodeBase
{
    static_assert(!Const, "not possible on ConstNode");

    int const lweight = weight();
    if(lweight <= 0){
        auto it = m_tree->m_tree.emplace(std::next(m_it), lweight - 1, std::move(child));

        m_it = std::prev(it);
        m_it->first = 1;
        return {m_tree, it};
    }

    auto endIt = end().m_it;
    int const dweight = deep_weight();

    auto it = m_tree->m_tree.emplace(endIt, dweight - 1, std::move(child));

    auto prevIt = std::prev(it);
    prevIt->first -= dweight - 1;

    NodeBase ret{m_tree, it};
    m_it = ret.parent().m_it;
    return ret;
}

/**
    @return the node of the appended child
    @throw strong exception-garantee if MoveAssignation of T does not throw
    @note child is wiped from the source tree by child.remove()
**/
template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::append(NodeBase&& child) -> NodeBase
{
    static_assert(!Const, "not possible on ConstNode");

    auto childSrcBegIt = child.m_it;
    auto childSrcEndIt = child.end().m_it;
    auto childSrcSize = std::distance(childSrcBegIt, childSrcEndIt);
    auto childSrcDeepWeight = child.deep_weight();

    int const lweight = weight();
    if(lweight <= 0){
        auto it = m_tree->m_tree.insert(std::next(m_it),
                                        std::make_move_iterator(childSrcBegIt),
                                        std::make_move_iterator(childSrcEndIt));
        child.remove();

        auto lastChildDestIt = std::next(it, childSrcSize - 1);
        lastChildDestIt->first = lastChildDestIt->first - childSrcDeepWeight + lweight - 1;

        m_it = std::prev(it);
        m_it->first = 1;
        return {m_tree, it};
    }

    auto endIt = end().m_it;
    int const dweight = deep_weight();

    auto it = m_tree->m_tree.insert(endIt,
                                    std::make_move_iterator(childSrcBegIt),
                                    std::make_move_iterator(childSrcEndIt));
    child.remove();

    auto lastChildDestIt = std::next(it, childSrcSize - 1);
    lastChildDestIt->first = lastChildDestIt->first - childSrcDeepWeight + dweight - 1;

    auto prevIt = std::prev(it);
    prevIt->first -= dweight - 1;

    NodeBase ret{m_tree, it};
    m_it = ret.parent().m_it;
    return ret;
}

/**
    @throw strong exception-guarantee
**/
template<class T> template<bool Const>
void ParseTree<T>::NodeBase<Const>::clear_children()
{
    auto dweight = deep_weight();
    m_tree->m_tree.erase(begin().m_it, end().m_it);
    m_it->first = dweight;
}

/**
    @return the node following the removed element
    @throw strong exception-guarantee
**/
template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::remove() -> NodeBase
{
    auto dweight = deep_weight();
    auto it = m_tree->m_tree.erase(m_it, end().m_it);
    if(it != m_tree->m_tree.begin() && dweight != 0){
        auto before = std::prev(it);
        before->first += dweight;
    }
    return {m_tree, it};
}

/**
    @throw strong exception-guarantee
**/
template<class T> template<bool Const>
void ParseTree<T>::NodeBase<Const>::skip_remove()
{
    auto lweight = weight();
    if(lweight <= 0){
        remove();
        return;
    }
    auto dsize = deep_size();
    auto frontIt = m_tree->m_tree.erase(m_it);
    std::advance(frontIt, dsize - 1);
    frontIt->first -= lweight;
}

template<class T> template<bool Const>
void ParseTree<T>::NodeBase<Const>::prune(NodeBase& other)
{
    auto endIt = end().m_it;
    auto otherEndIt = other.end().m_it;

    int const dweight = deep_weight();
    int const otherDeepWeight = other.deep_weight();

    auto it = std::move(other.m_it, otherEndIt, m_it);
    auto resizeIt = std::move(endIt, m_tree->m_tree.end(), it);
    m_tree->resize(std::distance(m_tree->begin(), resizeIt));

    std::prev(it)->first += otherDeepWeight - dweight;
    other.m_it = m_it;
}

/// Private

template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::weight() const
{
    return m_it->first;
}

template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::deep_weight() const
{
    return std::accumulate(begin().m_it, end().m_it, weight(), [](auto& init, auto& p){
        return init + p.first;
    });
}
