#pragma once
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include "BPtree.h"
#include "GameWorld.h"

#include <cstdint>
#include <utility>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& insertion) {
    os << '{';
    auto it = insertion.begin();
    for (size_t i = 0, N = insertion.size(); i < N; i++) {
        os << *it;
        if (i < (N - 1)) os << ", ";
        it++;
    }
    return os << '}';
}

#include <unordered_set>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& insertion) {
    os << '{';
    auto it = insertion.begin();
    for (size_t i = 0, N = insertion.size(); i < N; i++) {
        os << *it;
        if (i < (N - 1)) os << ", ";
        it++;
    }
    return os << '}';
}

template<typename T, typename V>
std::ostream& operator<<(std::ostream& os, const std::pair<T, V>& insertion) {
    return os << "<" << insertion.first << ", " << insertion.second << ">";
}

template<typename T, typename K>
std::ostream& operator<<(std::ostream& os, const std::map<T, K>& insertion) {
    os << '{';
    auto it = insertion.begin();
    for (size_t i = 0, N = insertion.size(); i < N; i++) {
        os << it->first << ">" << it->second;
        if (i < (N - 1)) os << ", ";
        it++;
    }
    return os << '}';
}

namespace detail
{


    template<std::size_t num_bytes>
    struct unsigned_integer;

    template<>
    struct unsigned_integer<1>
    {
        using type = std::uint8_t;
    };

    template<>
    struct unsigned_integer<2>
    {
        using type = std::uint16_t;
    };

    template<>
    struct unsigned_integer<4>
    {
        using type = std::uint32_t;
    };

    template<>
    struct unsigned_integer<8>
    {
        using type = std::uint64_t;
    };

    template<std::size_t num_bytes>
    using unsigned_integer_t = typename unsigned_integer<num_bytes>::type;


} // end detail


template<class UnsignedInteger>
std::pair<
    detail::unsigned_integer_t<sizeof(UnsignedInteger) / 2>,
    detail::unsigned_integer_t<sizeof(UnsignedInteger) / 2>
>
decode_morton_2d(UnsignedInteger m)
{
    using half_width_type = detail::unsigned_integer_t<sizeof(UnsignedInteger) / 2>;

    half_width_type x = 0;
    half_width_type y = 0;

    // for each bit of the half width numbers
#pragma unroll
    for (int i = 0; i < sizeof(half_width_type) * 8; ++i)
    {
        // pick out the ith even bit of the full width integer and assign it to bit i
        x |= (m & UnsignedInteger(1) << 2 * i) >> i;

        // pick out the ith odd bit of the full width integer and assign it to bit i+1
        y |= (m & UnsignedInteger(1) << 2 * i + 1) >> (i + 1);
    }

    return std::make_pair(x, y);
}


template<class UnsignedInteger>
detail::unsigned_integer_t<2 * sizeof(UnsignedInteger)>
encode_morton_2d(UnsignedInteger x, UnsignedInteger y)
{
    using double_width_type = detail::unsigned_integer_t<2 * sizeof(UnsignedInteger)>;

    double_width_type result = 0;

    // for each bit of the double width number
    for (int i = 0; i < sizeof(double_width_type) * 8; ++i)
    {
        result |= (x & double_width_type(1) << i) << i | (y & double_width_type(1) << i) << (i + 1);
    }

    return result;
}


template<class UnsignedInteger>
detail::unsigned_integer_t<2 * sizeof(UnsignedInteger)>
encode_morton_2d(std::pair<UnsignedInteger, UnsignedInteger> xy)
{
    return encode_morton_2d(xy.first, xy.second);
}

#include <map>
#include <unordered_set>

namespace NCL {
    namespace CSC8503 {


        enum CollisionDMethod {
            None,
            QuadTreeY,
            BpTree_const,
            BpTree_NotConst,
            RedBlackTree_const,
            RedBlackTree

        };

        struct node;

        class PhysicsSystem {
        public:
            unsigned long long min_z_value;
            std::map<unsigned long long, std::unordered_set<GameObject*>> RBtree;
            frozenca::BTreeMap<unsigned long long, std::unordered_set<GameObject*>> bptree2;

            //node* bptree = nullptr;
            PhysicsSystem(GameWorld& g, CollisionDMethod NewDetectionMethode);
            ~PhysicsSystem();
            void finalise_initialisation();

            void Clear();

            void Update(float dt);

            void UseGravity(bool state) {
                applyGravity = state;
            }

            void SetGlobalDamping(float d) {
                globalDamping = d;
            }

            void SetGravity(const Vector3& g);
            void print_tree() const {
                for (const auto& [k, v] : RBtree) {
                    std::cout << k << " + {";
                    for (const auto& value : v)
                        std::cout << value << ",";
                    std::cout << "}" << std::endl;
                }
                std::cout << "End tree" << std::endl;
            }
            void SetDamping(const float& d);
            float GetDamping() const;
            void InsertGameObjectIntoBTree(GameObject* i, bool check_containment = false);

            bool RemoveGameObjectWithZValue(GameObject* gameObject, unsigned long long Z_value);

            GameWorld& GetGameWorld() { return gameWorld; }

            //red black
            void RedBlack_initialisation();

        protected:

            template <typename Map> inline void broadphasemap(Map& map, bool doFinalise) {
                broadphaseCollisions.clear();
                collisions_being_checked.clear();
                if (doFinalise) 
                    finalise_initialisation();

                std::pair<GameObject*, GameObject*> cp;
                for (const auto& [k, v] : map) {
                    if (v.size() > 1) {
                        std::vector<GameObject*> tmp{ v.begin(), v.end() };
                        CollisionDetection::CollisionInfo info;
                        for (int j = 0; j < v.size(); j++)
                            for (int k = 0; k < j; k++) {
                                info.a = cp.first = std::min(tmp.at(k), tmp.at(j));
                                info.b = cp.second = std::max(tmp.at(k), tmp.at(j));
                                if (collisions_being_checked.insert(cp).second)
                                    broadphaseCollisions.insert(info);
                            }
                    }
                }

                if (doFinalise)
                    map.clear();
            }

            unsigned long long CalculateZValue(float xPoint, float yPoint) {
                // Scale and truncate the floating-point coordinates to integers
                uint32_t scaledX = static_cast<uint32_t>(xPoint );
                uint32_t scaledY = static_cast<uint32_t>(yPoint );

                // Call encode_morton_2d with the scaled integer coordinates
                return encode_morton_2d(scaledX, scaledY);
            }

            bool Contains(GameObject* object, Vector3 halfDims, unsigned long long Z_value);

            std::set<std::pair<GameObject*, GameObject*>> collisions_being_checked;
            void BasicCollisionDetection();
            void BroadPhaseQuadTree();

            void BroadPhaseBppTree();

            void BroadPhaseNotConstantBppTree();

            void NarrowPhase();

            void ClearForces();

            void IntegrateAccel(float dt);
            void IntegrateVelocity(float dt);

            void UpdateConstraints(float dt);

            void UpdateCollisionList();
            void UpdateObjectAABBs();

            void ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
            //red black
            void InsertGameObjectIntoRBTree(GameObject* gameObject, bool checkContainment = false);
            bool RemoveGameObjectWithZZValue(GameObject* gameObject, unsigned long long Z_value);
            void BroadPhaseConstantRBTree();

            GameWorld& gameWorld;

            bool	applyGravity;
            Vector3 gravity;
            float	dTOffset;
            float	globalDamping;
            float	damping;

            std::set<CollisionDetection::CollisionInfo> allCollisions;
            std::set<CollisionDetection::CollisionInfo> broadphaseCollisions;
            std::vector<CollisionDetection::CollisionInfo> broadphaseCollisionsVec;
            bool useBroadPhase = true;
            int numCollisionFrames = 5;
            CollisionDMethod DetectionMethode;
        };
    }
}