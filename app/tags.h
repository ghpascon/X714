struct TagItem
{
    String epc;
    String tid;
    byte ant;
    byte rssi;
    bool protected_tag;
};

class Tags
{
public:
    std::vector<TagItem> items;

    // Adiciona um novo tag se tid não existir
    void add(const String &epc, const String &tid, byte ant, byte rssi, bool protected_tag)
    {
        for (const auto &item : items)
        {
            if (item.tid == tid)
                return; // já existe
        }
        items.push_back({epc, tid, ant, rssi, protected_tag});
    }

    // Retorna todos os tags
    std::vector<TagItem> get() const
    {
        return items;
    }

    // Limpa todos os tags
    void clear()
    {
        items.clear();
    }
};