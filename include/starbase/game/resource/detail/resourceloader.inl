template<class T>
ResourcePtr<T> ResourceLoader::Get(const id_t id)
{
	const auto tId = TypedResId::Of<T>(id);
	if (m_resourcePtrs.count(tId) && !m_resourcePtrs[tId].expired()) {
		std::weak_ptr<const IResource>& weakPtr = m_resourcePtrs[tId];
		std::shared_ptr<const IResource> ptr(weakPtr);
		return ResourcePtr<T>(id, std::move(std::static_pointer_cast<const T>(ptr)));
	}
	else {
		std::shared_ptr<const T> placeholder = T::Placeholder();
		return ResourcePtr<T>(id, std::move(placeholder));
	}
}

template<class T>
ResourcePtr<T> ResourceLoader::Load(const id_t id)
{
	const auto tId = TypedResId::Of<T>(id);
	if (m_resourcePtrs.count(tId) && !m_resourcePtrs[tId].expired()) {
		return Get<T>(id);
	}
	else {
		std::shared_ptr<const T> ptr = T::Create(id, m_filesystem);
		if (ptr) {
			std::shared_ptr<const IResource> abstractPtr = std::static_pointer_cast<const IResource>(ptr);
			m_resourcePtrs[tId] = std::weak_ptr<const IResource>(abstractPtr);
			return ResourcePtr<T>(id, std::move(ptr));
		}
		else {
			std::shared_ptr<const T> placeholder = T::Placeholder();
			return ResourcePtr<T>(id, std::move(placeholder)); // don't add it to resourcePtrs!
		}
	}
}

template<class T>
ResourcePtr<T> ResourceLoader::Load(const id_t id, const id_t cacheId)
{
	auto ret = Load<T>(id);
	m_cachePtrs[cacheId].push_back(ret);
	return std::move(ret);
}

template<class T>
void ResourceLoader::Preload(id_t id, id_t cacheId)
{
	Load<T>(id, cacheId);
}

template<typename T>
ResourceLoader::TypedResId ResourceLoader::TypedResId::Of(id_t resourceId)
{
	return std::move(ResourceLoader::TypedResId(std::type_index(typeid(T)), resourceId));
}

ResourceLoader::TypedResId::TypedResId(std::type_index typeIndex, id_t resourceId)
	: typeIndex(typeIndex), resourceId(resourceId)
{}

bool ResourceLoader::TypedResId::operator==(const ResourceLoader::TypedResId& other) const
{
	return typeIndex == other.typeIndex
		&& resourceId == other.resourceId;
}

int ResourceLoader::TypedResId::operator<(const ResourceLoader::TypedResId& other) const
{
	if (typeIndex < other.typeIndex) return 1;
	else if (typeIndex > other.typeIndex) return -1;
	else return resourceId - other.resourceId;
}

std::size_t ResourceLoader::TypedResId::Hash::operator()(const ResourceLoader::TypedResId& k) const
{
	return std::hash<std::type_index>()(k.typeIndex)
		^ std::hash<id_t>()(k.resourceId);
}
