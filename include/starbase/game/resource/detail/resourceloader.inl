template<class T>
std::shared_ptr<const T> ResourceLoader::Get(const std::uint32_t id)
{
	if (m_resourcePtrs.count(id) && !m_resourcePtrs[id].expired()) {
		std::weak_ptr<const IResource>& weakPtr = m_resourcePtrs[id];
		std::shared_ptr<const IResource> ptr(weakPtr);
		return std::move(std::static_pointer_cast<const T>(ptr));
	}
	else {
		std::shared_ptr<const T> placeholder = T::Placeholder();
		return std::move(placeholder);
	}
}

template<class T>
std::shared_ptr<const T> ResourceLoader::Load(const char* filename)
{
	const std::uint32_t id = ID(filename);
    if (m_resourcePtrs.count(id) && !m_resourcePtrs[id].expired()) {
		return Get<T>(id);
    } else {
        std::shared_ptr<const T> ptr = T::Create(filename, m_filesystem);
        if (ptr) {
			std::shared_ptr<const IResource> abstractPtr = std::static_pointer_cast<const IResource>(ptr);
			m_resourcePtrs[id] = std::weak_ptr<const IResource>(abstractPtr);
            return ptr;
        } else {
            std::shared_ptr<const T> placeholder = T::Placeholder();
            return std::move(placeholder); // don't add it to resourcePtrs!
        }
    }
}
