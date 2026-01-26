import unreal

def run(target_folder, prefix):
    """
    批量重命名函数的入口
    :param target_folder: UE内部路径，例如 "/Game/Props"
    :param prefix: 你想要的前缀，例如 "SM_"
    """
    
    # 1. 获取资产库工具
    editor_asset_lib = unreal.EditorAssetLibrary()

    # 2. 安全检查：确保路径有效
    if not editor_asset_lib.does_directory_exist(target_folder):
        unreal.log_error(f"❌ 找不到文件夹: {target_folder}")
        return

    # 3. 获取文件夹内的所有资产 (包括子文件夹 recursive=True)
    asset_list = editor_asset_lib.list_assets(target_folder, recursive=True, include_folder=False)
    
    if len(asset_list) == 0:
        unreal.log_warning(f"⚠️ 文件夹 {target_folder} 是空的！")
        return

    unreal.log(f"🚀 开始处理 {target_folder} 下的 {len(asset_list)} 个文件...")

    # 4. 开启一个进度条任务 (处理大量文件时防止界面卡死)
    with unreal.ScopedSlowTask(len(asset_list), f"正在添加前缀 {prefix}...") as task:
        task.make_dialog(True) # 显示进度条窗口
        
        count = 0
        for asset_path in asset_list:
            # 如果点击了取消按钮，停止运行
            if task.should_cancel():
                break
            
            task.enter_progress_frame(1)

            # 获取资产数据
            asset_data = editor_asset_lib.find_asset_data(asset_path)
            old_name = asset_data.asset_name
            package_path = asset_data.package_path

            # --- 核心逻辑 ---
            
            # 检查是否已经有这个前缀了 (避免变成 SM_SM_Chair)
            if str(old_name).startswith(prefix):
                continue
            
            # 构建新路径
            new_name = f"{prefix}{old_name}"
            new_path = f"{package_path}/{new_name}"

            # 执行重命名
            success = editor_asset_lib.rename_asset(asset_path, new_path)
            
            if success:
                count += 1
                
    # 5. 完成后的收尾
    unreal.log(f"✅ 完成！成功重命名了 {count} 个资产。")
    unreal.log("⚠️ 提示：请记得右键文件夹选择 'Fix Up Redirectors' (修复重定向器)。")