/*
 * Quadtree.h
 *
 *  Created on: 2013-5-13
 *      Author: Administrator
 */

#ifndef QUADTREE_H_
#define QUADTREE_H_

#include <common.h>
#include <PoolObject.h>

namespace zertcore { namespace mapped{
using namespace zertcore::base;

typedef double								length_type;
typedef Rectangle<coord_t>					rect_type;
typedef Circle<coord_t>						circle_type;

template <class FC>
location_t	getLocation(typename FC::ptr);

template <class FC, class Traits = SmartTraits<FC>, class Container = list<typename Traits::ptr> >
class Quadtree;
/**
 *
 * Quadtree<FC, Container>
 *
 */
template <class FC, class Traits = SmartTraits<FC>, class Container = list<typename Traits::ptr> >
class QuadtreeNode
	: public PoolObject<QuadtreeNode<FC, Traits, Container> >
{
	template <class T, class Tr, class C> friend class Quadtree;

	typedef QuadtreeNode<FC, Traits, Container>
											self;
public:
	typedef self&							reference;
	typedef const self&						const_reference;

	typedef self*							original_ptr;
	typedef const self*						const_original_ptr;

	typedef original_ptr					ptr;

	typedef FC								value_type;
	typedef typename Traits::ptr			value_ptr;

public:
	typedef Container						container_type;
	typedef typename Container::iterator	iterator;

public:
	typedef size_t							level_type;
	typedef size_t							value_size_type;

public:
	const static size_t MAX_CHILDREN_AMOUNT	= 4;
	const static size_t MAX_LEVEL			= 12;
	const static size_t MAX_VALUE_AMOUNT	= 5;
	const static size_t MIN_CHILDREN_VALUE_AMOUNT
											= 5;

public:
	enum {
		TYPE_NONE							= -1,
		TYPE_ROOT							= 0,
		TYPE_NODE							= 1,
		TYPE_LEAF							= 2,
	};
	enum {
		NONE								= -1,
		TOP_LEFT							= 0,
		TOP_RIGHT							= 1,
		BOTTOM_LEFT							= 2,
		BOTTOM_RIGHT						= 3,
	};

public:
	explicit QuadtreeNode() : children_amount_(0), level_(0), value_size_(0),
		rect_(), node_type_(TYPE_NONE), parent_(Null), parent_index_(NONE) {
		for (size_t i = 0; i < MAX_CHILDREN_AMOUNT; ++i) {
			children_[i] = Null;
		}
	}

public:
	void setParent(ptr parent, int index = NONE) {
		parent_ = parent;
		if (parent_)
			level_ = parent->level_ + 1;

		if (index != NONE) {
			setParentIndex(index);
		}
	}
	void setParentIndex(int index) {
		parent_index_ = index;

		if (parent_) {
			calculateRectangle();
		}
	}
	void calculateRectangle() {
		if (!parent_)
			return ;
		/**
		 * calculate the its rectangle
		 */
		rect_.width = parent_->rect_.width / 2;
		rect_.height = parent_->rect_.height / 2;

		switch (parent_index_) {
		case TOP_LEFT:
			rect_.x = parent_->rect_.x;
			rect_.y = parent_->rect_.y;
			break;
		case TOP_RIGHT:
			rect_.x = parent_->rect_.x + rect_.width;
			rect_.y = parent_->rect_.y;
			break;
		case BOTTOM_LEFT:
			rect_.x = parent_->rect_.x;
			rect_.y = parent_->rect_.y + rect_.height;
			break;
		case BOTTOM_RIGHT:
			rect_.x = parent_->rect_.x + rect_.width;
			rect_.y = parent_->rect_.y + rect_.height;
			break;
		}
	}

	bool add(value_ptr value, bool add_check = true) {
		if (!contain(value->template getLocation())) {
			return false;
		}

		if (children_amount_ > 0) {
			for (size_t i = 0; i < MAX_CHILDREN_AMOUNT; ++i) {
				if (children_[i] && children_[i]->contain(value->template getLocation())) {
					return children_[i]->add(value);
				}
			}
			printf("weird add failed.");
			return false;
		}

		if (add_check && value->template isAdded()) {
			printf("is added\n");
			return false;
		}

		/**
		 * this node is the branch end
		 */
		if (MAX_LEVEL && level_ >= MAX_LEVEL) {
			addValue(value);
			return true;
		}
		else {
			addValue(value);
			if (getValueAmount() > MAX_VALUE_AMOUNT) {
				divide();
			}
		}

		return true;
	}

	bool divide() {
		if (children_amount_ > 0)
			return false;

		setNode();
		children_amount_ = MAX_CHILDREN_AMOUNT;

		for (size_t i = 0; i < MAX_CHILDREN_AMOUNT; ++i) {
			children_[i] = load<QuadtreeNode>();
			children_[i]->setParent(this, i);
			children_[i]->setLeaf();

			for (iterator it = value_container_.begin();
				it != value_container_.end(); ) {
				if (children_[i]->add(*it, false)) {
					value_container_.erase(it++);
				}
				else {
					++it;
				}
			}
		}

		value_container_.clear();
		return true;
	}

	bool gather() {
		if (children_amount_ == 0)
			return false;

		setLeaf();
		children_amount_ = 0;
		for (size_t i = 0; i < MAX_CHILDREN_AMOUNT; ++i) {
			if (!children_[i])
				continue;

			if (!children_[i]->isNodeType(TYPE_LEAF)) {
				children_[i]->gather();
			}
			container_type& value_container = children_[i]->value_container_;

			for (iterator it = value_container.begin();
				it != value_container.end(); ++it) {
				add(*it, false);
			}

			del(children_[i]);
			children_[i] = Null;
		}
		tryGather();

		return true;
	}

	bool remove(value_ptr value) {
		bool ret = removeValue(value);
		tryGather();

		if (!ret) printf("remove failed\n");

		return ret;
	}

	bool update(value_ptr value) {
		bool ret = updateValue(value);
		tryGather();

		if (!ret) printf("update failed\n");

		return ret;
	}

	bool contain(const location_t& location) const {
		return rect_.inside(location);
	}
	bool intersect(const rect_type& rect) const {
		return rect_.intersect(rect);
	}
	bool intersect(const circle_type& circle) const {
		return rect_.intersect(circle);
	}
	bool intersect(value_ptr value) const {
		return rect_.intersect(value->getRectangle());
	}

public:
	level_type getLevel() const {
		return level_;
	}
	void setRectangle(const rect_type& rect) {
		rect_ = rect;
	}
	rect_type& getRectangle() {
		return rect_;
	}
	const rect_type& getRectangle() const {
		return rect_;
	}

protected:
	void calValueSize(int size) {
		value_size_ += size;
		if (parent_) {
			parent_->calValueSize(size);
		}
	}

	value_size_type getValueSize() const {
		return value_size_;
	}

protected:
	bool tryGather() {
		/**
		 * cant remove root node
		 */
		if (!parent_) {
			return false;
		}

		if (!isNodeType(TYPE_LEAF)) {
			printf("Try to gather node\n");
			return false;
		}

		if (parent_->getValueSize() <= MIN_CHILDREN_VALUE_AMOUNT) {
			return parent_->gather();
		}

		return true;
	}

	bool updateValue(value_ptr value) {
		if (value->template getQuadNode() != this)
			return false;
		/**
		 * if the location doesnt move far, just keep the in the node.
		 */
		if (contain(value->template getLocation())) {
			return true;
		}
		removeValue(value);
		if (parent_ && parent_->add(value))
			return true;

		return false;
	}
	bool addValue(value_ptr value) {
		ptr node = value->template getQuadNode();
		if (node) {
			node->calValueSize(-1);
		}

		value->template setQuadtreeIndex(value_container_.insert(value_container_.end(), value));
		value->template setQuadNode(this);

		calValueSize(1);

		return true;
	}
	void addValues(container_type& container) {
		for (iterator it = container.begin(); it != container.end(); ++it) {
			add(*it, false);
		}
	}
	bool removeValue(value_ptr value) {
		if (value->template getQuadNode() != this)
			return false;

		value_container_.erase(value->template getQuadtreeIndex());
		value->template setQuadNode(Null);

		calValueSize(-1);

		return true;
	}
	size_t getValueAmount() const {
		return value_container_.size();
	}
	void getValues(container_type& container) {
		container.insert(container.end(), value_container_.begin(),
			value_container_.end());
	}
	void setRoot() {
		node_type_ = TYPE_ROOT;
	}
	void setNode() {
		if (node_type_ != TYPE_ROOT)
			node_type_ = TYPE_NODE;
	}
	void setLeaf() {
		if (node_type_ != TYPE_ROOT)
			node_type_ = TYPE_LEAF;
	}
	int getNodeType() const {
		return node_type_;
	}
	bool isNodeType(int type) const {
		return node_type_ == type;
	}

public:
	void print(const string& sep) {
		if (children_amount_ > 0) {
			if (getValueAmount() > 0) {
				printf("Error NODE\n");
				return ;
			}

			for (size_t i = 0; i < MAX_CHILDREN_AMOUNT; ++i) {
				if (!children_[i]) {
					printf("[%d] is empty\n", i);
					continue;
				}

				children_[i]->print(sep + "--");
				printf("\n");
			}
		}
		else {
			printf("%s[%d][t:%d]", sep.c_str(), getValueAmount(), getNodeType());
			for (iterator it = value_container_.begin();
					it != value_container_.end(); ++it) {
				printf("(%f, %f)", (*it)->getLocation().x, (*it)->getLocation().y);
			}
			printf("\n");
		}

	}

public:
	template <typename T>
	size_t getScopeValues(const T& scope, container_type& container, size_t max = 0) {
		size_t amount = 0;
		if (children_amount_ > 0) {
			for (size_t i = 0; i < MAX_CHILDREN_AMOUNT; ++i) {
				if (!children_[i]) {
					continue;
				}

				if (max && container.size() >= max)
					return amount;

				if (children_[i]->intersect(scope)) {
					amount += children_[i]->getScopeValues(scope, container, max);
				}
			}
		}
		else {
			for (iterator it = value_container_.begin();
					it != value_container_.end(); ++it) {
				if (scope.template inside((*it)->getLocation())) {
					if (max && container.size() >= max)
						return amount;

					container.push_back((*it));
					amount++;
				}
			}

		}

		return amount;
	}

public:
	int getQuadIndex(const location_t& location) const {
		if (rect_.inside(location)) {
			if (location.x < rect_.x + rect_.width / 2) {
				if (location.y < rect_.y + rect_.height / 2) {
					return TOP_LEFT;
				}
				return BOTTOM_LEFT;
			}
			else {
				if (location.y < rect_.y + rect_.height / 2) {
					return TOP_RIGHT;
				}
				return BOTTOM_RIGHT;
			}
		}

		return NONE;
	}

private:
	int							children_amount_;
	level_type					level_;
	value_size_type				value_size_;
	rect_type					rect_;
	int							node_type_;

private:
	container_type				value_container_;

private:
	ptr							children_[MAX_CHILDREN_AMOUNT];
	ptr							parent_;
	int							parent_index_;
};

/**
 * QuadtreeValue<FC, Container>
 */
template <class FC, class Traits = SmartTraits<FC>, class Container = list<typename Traits::ptr> >
class QuadtreeValue
{
public:
	typedef Container						container_type;
	typedef typename Container::iterator	iterator;
public:
	typedef typename QuadtreeNode<FC, Traits, Container>::ptr
											node_ptr;
public:
	QuadtreeValue() : is_added_(false)	{;}

public:
	bool isAdded() const {
		return is_added_;
	}

public:
	void setQuadtreeIndex(iterator index) {
		index_ = index;
	}
	iterator getQuadtreeIndex() {
		return index_;
	}
	void setQuadNode(node_ptr node) {
		node_ = node;

		if (!node)
			is_added_ = false;
		else
			is_added_ = true;
	}
	node_ptr getQuadNode() {
		return node_;
	}
	location_t& getLocation();

private:
	iterator					index_;
	node_ptr					node_;

	bool						is_added_;
};

/**
 *
 * Quadtree<FC, Container>
 *
 */
template <class FC, class Traits, class Container>
class Quadtree
{
	typedef Quadtree<FC, Traits, Container>	self;

public:
	typedef self&							reference;
	typedef const self&						const_reference;

	typedef self*							original_ptr;
	typedef const self*						const_original_ptr;

public:
	typedef FC								value_type;
	typedef typename Traits::ptr			value_ptr;

	typedef Container						container_type;
	typedef typename Container::iterator	iterator;

public:
	typedef QuadtreeNode<FC, Traits, Container>
											node_type;
	typedef typename node_type::ptr			node_ptr;

public:
	Quadtree(const rect_type& rect) : root_(), rect_(rect) {
		root_.template setRectangle(rect);
		root_.template setRoot();
	}
	Quadtree() : root_() {
		root_.template setRoot();
	}

	virtual ~Quadtree() {
		;
	}

public:
	void setRectangle(const rect_type& rect) {
		rect_ = rect;
		root_.template setRectangle(rect);
	}
	bool addValue(value_ptr value) {
		return root_.template add(value);
	}
	bool updateValue(value_ptr value) {
		node_ptr node = value->template getQuadNode();
		if (!node) return false;
		if (!node->template update(value)) {
			return root_.template add(value);
		}

		return true;
	}

	bool removeValue(value_ptr value) {
		node_ptr node = value->template getQuadNode();
		if (!node) return false;

		return node->template remove(value);
	}

	template <typename T>
	bool queryValues(const T& scope, container_type& container, size_t max = 0) {
		return root_.template getScopeValues(scope, container, max) > 0;
	}

	void print() {
		printf("size:%d\n", root_.getValueSize());
		root_.template print(string());
	}

private:
	node_type					root_;
	rect_type					rect_;
};


}}

#endif /* QUADTREE_H_ */
