import waifu2x_vulkan
from pathlib import Path

waifu2x_vulkan_path = Path(waifu2x_vulkan.__file__).parent

models_path = waifu2x_vulkan_path / "models"
datas = [(str(models_path), "waifu2x_vulkan/models")]
