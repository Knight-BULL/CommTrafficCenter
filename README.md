# CommTrafficCenter

# 生成SSH密钥，替换为你的GitHub邮箱
ssh-keygen -t ed25519 -C "your_email@example.com"
# 按回车默认保存到~/.ssh/id_ed25519，无需设置密码（也可设置，按需选择）
# Linux/Mac
cat ~/.ssh/id_ed25519.pub
# Windows（PowerShell）
type $HOME\.ssh\id_ed25519.pub
# 先查看当前远程地址（确认是HTTPS格式）
git remote -v
# 替换为SSH地址（格式：git@github.com:用户名/仓库名.git）
git remote set-url origin git@github.com:Knight-BULL/CommTrafficCenter.git
# 验证是否生效
git remote -v
#测试 SSH 连接
ssh -T git@github.com
remote: Invalid username or token. Password authentication is not supported for Git operations.
fatal: Authentication failed for 'https://github.com/Knight-BULL/CommTrafficCenter.git/'

# 直接删除 known_hosts 中所有 github.com 相关的条目（安全无副作用）
ssh-keygen -R github.com
ssh -T git@github.com