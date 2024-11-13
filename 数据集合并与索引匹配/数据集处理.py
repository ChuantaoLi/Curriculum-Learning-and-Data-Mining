# 取出海洋温度数据集的经纬度坐标
# 数据集路径是：1929-2022年全球站点的逐日平均风速数据\海洋温度数据\2004，需要先对每年的压缩包解压，里面放置了多个csv文件，每个csv文件都是一个观测站点的观测数据
# 因此，每个csv文件里面的经纬度坐标都是一样的
# 为了方便后面的处理，我们首先得把每年每个观测站点的坐标取出来放置在一个新的文件里面
# 需要注意的是，是每年每年地去提取，我们需要做的是2004年至2023年
import pandas as pd
import os

# 指定文件夹路径
folder_path = r"D:\Git_code\数据集合并\2005"
output_file = r"D:\Git_code\数据集合并\unique_lat_lon_2005.csv"  # 输出文件路径

unique_lat_lon = pd.DataFrame(columns=['LATITUDE', 'LONGITUDE'])

# 遍历文件夹中的每个CSV文件
for filename in os.listdir(folder_path):
    if filename.endswith('.csv'):
        # 构建文件路径
        file_path = os.path.join(folder_path, filename)

        # 读取CSV文件
        df = pd.read_csv(file_path)

        # 提取唯一的LATITUDE和LONGITUDE
        unique_values = df[['LATITUDE', 'LONGITUDE']].drop_duplicates()

        # 将唯一值追加到结果DataFrame
        unique_lat_lon = pd.concat([unique_lat_lon, unique_values], ignore_index=True)

# 去重并去掉包含空值的行
unique_lat_lon = unique_lat_lon.drop_duplicates().dropna()
unique_lat_lon['index'] = range(1, len(unique_lat_lon) + 1)
unique_lat_lon.to_csv(output_file, index=False)

print(f"唯一的LATITUDE和LONGITUDE已成功导出到 {output_file}")

# 台风路径与观测站匹配
# 在得到海洋温度数据集的观测站经纬度后，我们需要对附件给的数据集的台风路径经纬度匹配一个最相近的观测站点
# 需要注意的是，是每年每年地去匹配，我这里的代码是2004年的，可以看到文件名 2004年台风数据集.xlsx 是我手动对 cleaned_1945-2023.xlsx 按年份截取的
import pandas as pd
import numpy as np

# 读取台风数据和观测站数据
typhoon_data = pd.read_excel(r'D:\Git_code\数据集合并\2005年台风数据集.xlsx')
stations_data = pd.read_csv(r'D:\Git_code\数据集合并\unique_lat_lon_2005.csv')

# 提取观测站的经纬度和索引
station_coords = stations_data[['LONGITUDE', 'LATITUDE']].values
station_indices = stations_data['index'].values

# 创建用于存储最近观测站经纬度和index的列表
nearest_station_longitudes = []
nearest_station_latitudes = []
nearest_station_indices = []


# 定义 Haversine 公式计算球面距离
def haversine(lon1, lat1, lon2, lat2):
    # 将度数转换为弧度
    lon1, lat1, lon2, lat2 = map(np.radians, [lon1, lat1, lon2, lat2])

    # 计算差值
    dlon = lon2 - lon1
    dlat = lat2 - lat1

    # Haversine 公式
    a = np.sin(dlat / 2) ** 2 + np.cos(lat1) * np.cos(lat2) * np.sin(dlon / 2) ** 2
    c = 2 * np.arcsin(np.sqrt(a))

    # 地球半径，单位为公里（6371 公里）
    r = 6371
    return c * r


# 遍历台风数据中的每个样本
for index, typhoon_row in typhoon_data.iterrows():
    typhoon_lon = typhoon_row['LONGITUDE']
    typhoon_lat = typhoon_row['LATITUDE']

    # 计算台风点与所有观测站之间的 Haversine 距离
    distances = np.array([haversine(typhoon_lon, typhoon_lat, station_lon, station_lat)
                          for station_lon, station_lat in station_coords])

    # 找到最小距离对应的观测站索引
    nearest_idx = np.argmin(distances)

    # 获取最近观测站的经纬度和索引
    nearest_station_longitudes.append(station_coords[nearest_idx][0])
    nearest_station_latitudes.append(station_coords[nearest_idx][1])
    nearest_station_indices.append(station_indices[nearest_idx])

# 将最近观测站的经纬度和index添加到台风数据中
typhoon_data['NEAREST_STATION_LONGITUDE'] = nearest_station_longitudes
typhoon_data['NEAREST_STATION_LATITUDE'] = nearest_station_latitudes
typhoon_data['NEAREST_STATION_INDEX'] = nearest_station_indices

typhoon_data.to_excel('2005年台风与观测站点匹配数据集.xlsx', index=False)

# 经过上述的处理，2004年台风与观测站点匹配数据集.xlsx 这个数据集相对于 2004年台风数据集.xlsx 多了三列，分别是匹配的观测站点经度 纬度和所属年份的海洋温度数据集文件夹的csv文件索引
# 接下来需要通过索引去找到对应的csv文件，然后就可以把海洋气象数据和附件数据合并在一起了
import pandas as pd
import os

data_folder = r"D:\Git_code\数据集合并\2005"
matching_data_path = '2005年台风与观测站点匹配数据集.xlsx'

# 读取台风与观测站点匹配数据集
matching_data = pd.read_excel(matching_data_path)

# 对匹配数据集的“当前台风日期”进行日期格式的处理，转换成 'y-m-d' 格式
matching_data[['当前台风日期', '当前台风时间']] = matching_data['当前台风时间'].str.split('T', expand=True)
matching_data['当前台风日期'] = pd.to_datetime(matching_data['当前台风日期']).dt.strftime('%Y-%m-%d')

# 创建一个用于存储合并结果的 DataFrame
merged_results = pd.DataFrame()

# 获取指定文件夹中的所有 CSV 文件，并按名称排序
csv_files = sorted([f for f in os.listdir(data_folder) if f.endswith('.csv')])

# 遍历匹配数据集中的每一行
for idx, row in matching_data.iterrows():
    typhoon_date = row['当前台风日期']
    station_index = int(row['NEAREST_STATION_INDEX'])  # 确保station_index是整数

    # 根据station_index获取对应的CSV文件名（注意索引从0开始）
    if 0 <= station_index < len(csv_files):
        station_filename = csv_files[station_index]
        station_csv_path = os.path.join(data_folder, station_filename)

        # 调试：打印当前处理的台风日期和观测站文件名
        print(f"Processing typhoon date: {typhoon_date}, Station file: {station_filename}")

        # 检查文件是否存在
        if os.path.exists(station_csv_path):
            # 读取该观测站的 CSV 文件
            try:
                station_data = pd.read_csv(station_csv_path)
                print(f"Successfully read data from: {station_csv_path}")
            except Exception as e:
                print(f"Failed to read {station_csv_path}: {e}")
                continue

            # 将观测站的日期转换为统一的格式，确保与台风日期格式一致
            station_data['DATE'] = pd.to_datetime(station_data['DATE']).dt.strftime('%Y-%m-%d')

            # 查找与当前台风时间相同日期的样本
            matched_row = station_data[station_data['DATE'] == typhoon_date]

            # 调试：打印是否找到匹配的样本
            if matched_row.empty:
                print(f"No data found for date: {typhoon_date}")
            else:
                print(f"Found matching data for date: {typhoon_date}")

            # 如果找到匹配的样本，则将其与台风数据的当前行合并
            if not matched_row.empty:
                # 使用 pd.concat 合并行数据
                merged_row = pd.concat([row.to_frame().T, matched_row.reset_index(drop=True)], axis=1)

                # 将合并后的行添加到结果 DataFrame
                merged_results = pd.concat([merged_results, merged_row], ignore_index=True)
    else:
        print(f"Invalid index: {station_index}. It should be between 0 and {len(csv_files) - 1}")

# 保存合并后的结果数据集
output_path = '2005年台风与观测站点合并数据集.xlsx'
merged_results.to_excel(output_path, index=False)

print(f"合并完成，结果已保存到 '{output_path}'")

# 上面的代码处理后的合并数据集有错行，指的是样本不是一一对应的，没关系，我们把两部分的空行依次去除，再合一起就行了
# 顺便去除一些重复的特征
df = pd.read_excel(r'D:\Git_code\数据集合并\2005年台风与观测站点合并数据集.xlsx')

# 对前17列去除空行
df_first_part = df.iloc[:, :18].dropna()
# df_first_part.to_excel(r'df_first_part.xlsx', index=False)

# 对从第18列到最后一列去除空行
df_second_part = df.iloc[:, 18:].dropna()
# df_second_part.to_excel(r'df_second_part.xlsx', index=False)

# 合并去除空行后的两个数据集
df_combined = pd.concat([df_first_part.reset_index(drop=True), df_second_part.reset_index(drop=True)], axis=1)
columns_to_drop = [
    # 'NEAREST_STATION_LONGITUDE', 'NEAREST_STATION_LATITUDE',
    'NEAREST_STATION_INDEX', 'STATION', 'DATE',
    'LATITUDE.1', 'LONGITUDE.1', 'NAME'
]

df_combined = df_combined.drop(columns=columns_to_drop)
df_combined.to_excel(r'D:\Git_code\数据集合并\2005年台风与观测站点合并数据集.xlsx', index=False)
print('大功告成！')
