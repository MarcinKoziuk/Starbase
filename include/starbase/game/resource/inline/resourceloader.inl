template<class T>
std::shared_ptr<const T> ResourceLoader::Load(const std::string& filename)
{
    if (m_resourcePtrs.count(filename) && !m_resourcePtrs[filename].expired()) {
        std::weak_ptr<const IResource> weakPtr = m_resourcePtrs[filename];
		std::shared_ptr<const IResource> ptr(weakPtr);
        return std::static_pointer_cast<const T>(ptr);
    } else {
        std::shared_ptr<const T> ptr = T::Create(filename, m_filesystem);
        if (ptr) {
			std::shared_ptr<const IResource> abstractPtr = std::static_pointer_cast<const IResource>(ptr);
			m_resourcePtrs[filename] = std::weak_ptr<const IResource>(abstractPtr);
            return ptr;
        } else {
            std::shared_ptr<const T> placeholder = T::Placeholder();
            return placeholder; // don't add it to resourcePtrs!
        }
    }
}
