# cmake/Hunter/passwords.cmake

hunter_upload_password(
    REPO_OWNER "huntercache"
    REPO "voxigen"
    USERNAME "huntercachebot"
    PASSWORD "$ENV{GITHUB_USER_PASSWORD}"
)