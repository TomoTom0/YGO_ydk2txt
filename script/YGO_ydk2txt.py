# setup
import os
import sys
import sqlite3
from pathlib import Path

orig_path=str(Path().resolve())

def mk_ls(keyword="", dir=""):
    if dir == "":
        dir = os.curdir
    files = [path for path in os.listdir(dir) if keyword in path]
    return files

# read ydk
def read_ydk(ydk_name, ADS_path):
    # deck path
    deck_dir_path = f"{ADS_path}/deck/"
    os.chdir(deck_dir_path)
    # read the content of .ydk file
    with open(ydk_name, "r") as f:
        deck_content = f.readlines()

    # exclude #main, #extra, !side
    check_keys = ["#main\n", "#extra\n", "!side\n"]  # at first
    check_key_indexes = [deck_content.index(s) for s in check_keys]

    # obtain card indexes
    cards_indexes = [s[:-1] for ind, s in enumerate(deck_content) if ind > check_key_indexes[0]
                     and ind < check_key_indexes[1]]

    cards_indexes_all = {}
    cards_indexes_all["main"] = cards_indexes
    cards_indexes_all["extra"] = [s[:-1] for ind, s in enumerate(deck_content) if ind > check_key_indexes[1]
                                  and ind < check_key_indexes[2]]
    cards_indexes_all["side"] = [s[:-1] for ind,
                                 s in enumerate(deck_content) if ind > check_key_indexes[2]]

    deck_size = check_key_indexes[1]-check_key_indexes[0]-1

    return cards_indexes, deck_size, cards_indexes_all

def obtain_names_Jap(ADS_path, cards_indexes):
    # obtain Japanese names

    cards_names_Jap = []
    for ind in cards_indexes:
        lua_names =\
            [f"{ADS_path}/script/official/c{ind}.lua",
             f"{ADS_path}/repositories/delta/script/official/c{ind}.lua",
             f"{ADS_path}/repositories/delta/script/delta/pre-release/c{ind}.lua",
             f"{ADS_path}/repositories/delta/script/delta/pre-errata/c{ind}.lua"]

        # default : failed case
        lua_content = [f"----CAN'T_FIND_c{ind}.lua--"]
        for lua_name in lua_names:
            try:
                with open(lua_name, "r") as f:
                    lua_content = f.readlines()
            except FileNotFoundError:
                pass
                continue
            else:
                break

        if not lua_content == [f"----CAN'T_FIND_c{ind}.lua--"]:
            cards_names_Jap.append(lua_content[0][2:-1])
            continue

        # for card with fixed pics
        ind = f"{int(ind)-1}"
        lua_names =\
            [f"{ADS_path}/script/official/c{ind}.lua",
             f"{ADS_path}/repositories/delta/script/official/c{ind}.lua",
             f"{ADS_path}/repositories/delta/script/delta/pre-release/c{ind}.lua",
             f"{ADS_path}/repositories/delta/script/delta/pre-errata/c{ind}.lua"]
        for lua_name in lua_names:
            try:
                with open(lua_name, "r") as f:
                    lua_content = f.readlines()
            except FileNotFoundError:
                pass
                continue
            else:
                break
        cards_names_Jap.append(lua_content[0][2:-1])

    return cards_names_Jap


def find_path(path_cand,  judge_dirs, purpose=""):
    ans_path = None
    for _ in range(5):
        list_tmp = mk_ls(dir=path_cand)
        if set(judge_dirs) <= set(list_tmp):
            ans_path = path_cand
            break
        path_cand += "/../"

    if ans_path is None:
        print(f"can't find {purpose} path")
        raise Error
    return ans_path


def ydk2txt_main(main_path=str(Path().resolve())):
    # find ADS_path
    ADS_path_cand = f"{main_path}/"
    judge_dirs = ["deck", "expansions", "replay"]
    ADS_path = find_path(ADS_path_cand, judge_dirs, "ADS")

    # find dir_path
    dir_path_cand = f"{main_path}/"
    judge_dirs = ["deck", "output", "script"]
    dir_path = find_path(dir_path_cand, judge_dirs, "ydk2txt_dir")

    deck_dir_path = f"{dir_path}/deck/"
    output_dir_path = f"{dir_path}/output/"

    # obtain ydk_names
    ydk_names = mk_ls(".ydk", dir=deck_dir_path)
    for ydk_name in ydk_names:
        # read ydk
        cards_indexes, deck_size, cards_indexes_all =\
            read_ydk(ydk_name=ydk_name, ADS_path=ADS_path)

        # obtain Japanese names
        cards_names_Jap_all =\
            {k: obtain_names_Jap(ADS_path, inds)
             for k, inds in cards_indexes_all.items()}

        # output to text
        txt_content =\
            "\n\n".join([
                f"# {k}\n"+"\n".join(v)
                for k, v in cards_names_Jap_all.items()])

        txt_name = f"{output_dir_path}/{ydk_name.replace('.ydk','.txt')}"
        with open(txt_name, mode="w") as f:
            f.write(txt_content)

if __name__ == "__main__":
    # execute main program
    ydk2txt_main(main_path=orig_path)