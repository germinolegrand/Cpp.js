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

        NodeValue& operator*(){ return get()->second; }
        NodeValue* operator->(){ return &get()->second; }
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
        NodeBase last_child() const;
        size_t children() const;
        size_t size() const{ return children(); }
        size_t deep_size() const;
        bool empty() const;
        bool contains(NodeBase<true>) const;
        int depth() const;
        int child_depth(NodeBase<true>) const;

        NodeBase append(T&& child);
        NodeBase append(T const& child);
        NodeBase append(NodeBase&& child);
        NodeBase prepend(T&& child);
        void clear_children();
        NodeBase remove();
        void skip_remove();
        void prune(NodeBase&);
        void wrap(T&& element);

        NodeBase(NodeBase<false> const& other): m_tree(other.m_tree), m_index(other.m_index) {}

    private:
        using Tree = std::conditional_t<Const,
            decltype(ParseTree::m_tree) const,
            decltype(ParseTree::m_tree)>;
        using TreeIterator = std::conditional_t<Const,
            typename Tree::const_iterator,
            typename Tree::iterator>;
        using TreeIndex = typename Tree::difference_type;

        Tree* m_tree;
        TreeIndex m_index;

        TreeIterator get() const { return get(m_index); }
        TreeIterator get(TreeIndex ind) const { return std::next(m_tree->begin(), ind); }
        TreeIndex get_index(TreeIterator it) const { return std::distance(m_tree->begin(), it); }

        friend class ParseTree;
        NodeBase(Tree& tree, TreeIndex index): m_tree(&tree), m_index(index) {}
        NodeBase(Tree* tree, TreeIndex index): m_tree(tree), m_index(index) {}

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
    void wrap(T&& element);
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
    return {m_tree, 0};
}

template<class T>
auto ParseTree<T>::root() const -> ConstNode
{
    return {m_tree, 0};
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

template<class T>
void ParseTree<T>::wrap(T&& element)
{
    root().wrap(std::forward<T>(element));
}


////////////////////////////////////
/// ParseTree::NodeBase
////////////////////////////////////

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::operator++() -> NodeBase&
{
    for(auto i = 0; (i += get(m_index++)->first) > 0;){

    }
    return *this;
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::operator--() -> NodeBase&
{
    for(auto i = 0; (i += get(--m_index)->first) < 0;){

    }
    return *this;
}

template<class T> template<bool Const>
bool ParseTree<T>::NodeBase<Const>::operator==(NodeBase const& other) const
{
    return m_tree == other.m_tree && m_index == other.m_index;
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::begin() const -> NodeBase
{
    return {m_tree, m_index + 1};
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
    return m_index == 0;
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::root() const -> NodeBase
{
    return {m_tree, 0};
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::parent() const -> NodeBase
{
    auto index = m_index;
    for(auto i = 0; (i += get(--index)->first) <= 0;){

    }
    return {m_tree, index};
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::previous_sibling() const -> NodeBase
{
    return std::prev(*this);
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::next_sibling() const -> NodeBase
{
    return std::next(*this);
}

/**
    @return last child if any, or *this if empty()
**/
template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::last_child() const -> NodeBase
{
    auto index = m_index;
    auto lastChildIndex = index;

    for(int i = 0; (i += get(index++)->first) > 0;){
        if(i == 1){
            lastChildIndex = index;
        }
    }
    return {m_tree, lastChildIndex};
}

template<class T> template<bool Const>
size_t ParseTree<T>::NodeBase<Const>::children() const
{
    auto index = m_index;

    size_t count = 0;

    for(int i = 0; (i += get(index++)->first) > 0;){
        if(i == 1){
            ++count;
        }
    }
    return count;
}

template<class T> template<bool Const>
size_t ParseTree<T>::NodeBase<Const>::deep_size() const
{
    return end().m_index - begin().m_index;
}

template<class T> template<bool Const>
bool ParseTree<T>::NodeBase<Const>::empty() const
{
    return get()->first < 1;
}

template<class T> template<bool Const>
bool ParseTree<T>::NodeBase<Const>::contains(NodeBase<true> other) const
{
    if(m_index >= other.m_index){
        return false;
    }

    return child_depth(other) > 0;
}

template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::depth() const
{
    return root().child_depth(*this);
}

/**
    @return depth > 0 if `child` is a descendent of `*this`
    @return 0 if `child` is `*this`
    @return anything < 0 if `child` is not a descendent of `*this`
**/
template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::child_depth(NodeBase<true> child) const
{
    int i = 0;
    auto index = m_index;
    for(i = get(index)->first; index != child.m_index && i > 0; i += get(++index)->first){

    }
    return i;
}



/// Manipulation

/**
    @note invalidates all iterators after insertion point
    @return the node of the appended child
    @throw strong exception-garantee
**/
template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::append(T&& child) -> NodeBase
{
    static_assert(!Const, "not possible on ConstNode");

    int const lweight = weight();
    if(lweight <= 0){
        auto it = m_tree->emplace(std::next(get()), lweight - 1, std::move(child));
        get()->first = 1;
        return {m_tree, get_index(it)};
    }

    auto endIndex = end().m_index;
    int const dweight = deep_weight();

    auto it = m_tree->emplace(get(endIndex), dweight - 1, std::move(child));

    auto prevIt = std::prev(it);
    prevIt->first -= dweight - 1;

    return {m_tree, get_index(it)};
}

/**
    @note invalidates all iterators after insertion point
    @return the node of the appended child
    @throw strong exception-garantee if MoveAssignation of T does not throw
    @note child is wiped from the source tree by child.remove()
**/
template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::append(NodeBase&& child) -> NodeBase
{
    static_assert(!Const, "not possible on ConstNode");

    auto childSrcBegIt = child.get(child.m_index);
    auto childSrcEndIt = child.get(child.end().m_index);
    auto childSrcSize = std::distance(childSrcBegIt, childSrcEndIt);
    auto childSrcDeepWeight = child.deep_weight();

    int const lweight = weight();
    if(lweight <= 0){
        auto it = m_tree->insert(std::next(get()),
                                 std::make_move_iterator(childSrcBegIt),
                                 std::make_move_iterator(childSrcEndIt));
        child.remove();

        auto lastChildDestIt = std::next(it, childSrcSize - 1);
        lastChildDestIt->first = lastChildDestIt->first - childSrcDeepWeight + lweight - 1;

        get()->first = 1;
        return {m_tree, get_index(it)};
    }

    auto endIt = get(end().m_index);
    int const dweight = deep_weight();

    auto it = m_tree->insert(endIt,
                             std::make_move_iterator(childSrcBegIt),
                             std::make_move_iterator(childSrcEndIt));
    child.remove();

    auto lastChildDestIt = std::next(it, childSrcSize - 1);
    lastChildDestIt->first = lastChildDestIt->first - childSrcDeepWeight + dweight - 1;

    auto prevIt = std::prev(it);
    prevIt->first -= dweight - 1;

    return {m_tree, get_index(it)};
}

template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::prepend(T&& child) -> NodeBase
{
    static_assert(!Const, "not possible on ConstNode");

    int const lweight = weight();

    auto it = m_tree->emplace(std::next(get()), lweight - 1, std::move(child));

    if(lweight <= 0){
        get()->first = 1;
    }
    return {m_tree, get_index(it)};
}

/**
    @throw strong exception-guarantee
**/
template<class T> template<bool Const>
void ParseTree<T>::NodeBase<Const>::clear_children()
{
    auto dweight = deep_weight();
    m_tree->erase(get(begin().m_index), get(end().m_index));
    get()->first = dweight;
}

/**
    @return the node following the removed element
    @throw strong exception-guarantee
**/
template<class T> template<bool Const>
auto ParseTree<T>::NodeBase<Const>::remove() -> NodeBase
{
    auto dweight = deep_weight();
    auto it = m_tree->erase(get(), get(end().m_index));
    if(it != m_tree->begin() && dweight != 0){
        auto before = std::prev(it);
        before->first += dweight;
    }
    return {m_tree, get_index(it)};
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
    auto frontIt = m_tree->m_tree.erase(get());
    std::advance(frontIt, dsize - 1);
    frontIt->first -= lweight;
}

template<class T> template<bool Const>
void ParseTree<T>::NodeBase<Const>::prune(NodeBase& other)
{
    auto endIt = get(end().m_index);
    auto otherEndIt = get(other.end().m_index);

    int const dweight = deep_weight();
    int const otherDeepWeight = other.deep_weight();

    auto it = std::move(get(other.m_index), otherEndIt, get());
    auto resizeIt = std::move(endIt, m_tree->end(), it);
    m_tree->resize(std::distance(m_tree->begin(), resizeIt));

    std::prev(it)->first += otherDeepWeight - dweight;
    other.m_index = m_index;
}

template<class T> template<bool Const>
void ParseTree<T>::NodeBase<Const>::wrap(T&& element)
{
    auto totalSize = deep_size() + 1;
    auto it = m_tree->emplace(get(), 1, std::forward<T>(element));
    std::next(it, totalSize)->first -= 1;
}

/// Private

template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::weight() const
{
    return get()->first;
}

template<class T> template<bool Const>
int ParseTree<T>::NodeBase<Const>::deep_weight() const
{
    return std::accumulate(get(begin().m_index), get(end().m_index), weight(), [](auto& init, auto& p){
        return init + p.first;
    });
}
