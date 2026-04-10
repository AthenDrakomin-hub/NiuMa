import re
with open('/workspace/test_niuma_cpp/server/Server/GrabNiuNiu/GrabNiuNiuRoom.cpp', 'r') as f:
    content = f.read()

# 1. 补上头文件
if '#include "Base/Log.h"' not in content:
    content = '#include "Base/Log.h"\n' + content

# 2. 替换所有错误的 getCard() 为 getCards() (只替换确实该改的地方)
# 那些是在 PokerGenre 或者 it->second 等调用处的
content = re.sub(r'genre\.getCard\(\)', 'genre.getCards()', content)
content = re.sub(r'pair\.second\.getCard\(\)', 'pair.second.getCards()', content)
content = re.sub(r'itCards->second\.getCard\(\)', 'itCards->second.getCards()', content)
content = re.sub(r'it->second\.getCard\(\)', 'it->second.getCards()', content)

# 3. 把 trySwapCardForPlayer 函数体直接替换掉，只保留 return false;
pattern = r'(bool GrabNiuNiuRoom::trySwapCardForPlayer\(const std::string\& playerId, int targetGenre\)\s*\{).*?(void GrabNiuNiuRoom::dealCardsToPlayer)'
replacement = r'\1\n    return false;\n}\n\n\2'
content = re.sub(pattern, replacement, content, flags=re.DOTALL)

with open('/workspace/test_niuma_cpp/server/Server/GrabNiuNiu/GrabNiuNiuRoom.cpp', 'w') as f:
    f.write(content)
