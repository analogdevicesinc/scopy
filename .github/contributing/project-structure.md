# Git Process

### Some resources

- https://opensource.com/article/18/6/anatomy-perfect-pull-request

#### When working on a new feature:
- Move to the starting point
- Resynchronize tree with remote
- Create new branch from starting branch

#### Example:

```bash
git checkout dev
git pull
git checkout -b <new_branch_name>
```

> [!NOTE]
> Prefixing branch name:
> - dev_ for branches starting from dev
> - ci_ for ci work
> - exp_ for experimental work – will never be merged back
> - v<x.xx.xx> - for releases and release candidates
>
> Example:
> ```bash
> git checkout -b dev_cursor_refactor origin/dev
> ```

- Implement feature – commit and push work as needed – refer to commit message structure
- Apply code styling - (usually done by running `./tools/format.sh`)
- Resynchronize with latest changes to starting point and push again. Example
```bash
git fetch
git rebase origin/dev
git push
```
- Create pull request
- Make sure CI tests pass. Assign reviewers.
- Discuss review, and implement changes as needed. Try to refrain from force pushing at this point as it makes everyones job a little harder. When finished, all the commits can be squashed/reorganized as needed
- Make sure PR is synchronized with starting point. Test feature before merge.
- Close PR - use only `Rebase and merge` or `Squash and merge`. Do not use Create merge commit

# Git commit message structure

Some resources:
- https://cbea.ms/git-commit/
- https://github.blog/2022-06-30-write-better-commits-build-better-projects/
- https://dhwthompson.com/2019/my-favourite-git-commit

A commit message should have the following structure:

```text
prefix/subprefix: short description (max 80 char)

longer description that explains why the change is needed as well as some
subtleties of the change. Can refer to other commit SHA as well as github
issues

Signed-off by: author.name <author.email@analog.com>
```

Refer to project specific documentation for module names and description

Some possible prefixes:

| Module name | Description                                                                                               |
|-------------|-----------------------------------------------------------------------------------------------------------|
| project     | A change in project structure – nonfunctional changes                                                     |
| tree        | Change that applies to many files in the project – such as styling, addressing a warning, etc             |
| cmake       | Changes related to general project cmake or cmake modules. (assuming cmake is the project config tool)    |
| ci          | CI and deployment                                                                                         |
| doc         | Add documentation or documentation infrastructure                                                         |
| tests       | Add tests or testing infrastructure                                                                       |
| toolsTools  | related to the project                                                                                    |
| general     | A change that affects multiple parts of Scopy – should not be used unless other module names do not apply |
| \<module>   | Project specific module                                                                                   |


# Default Branches

| Branch name        | Description                                                                                                        |
|--------------------|--------------------------------------------------------------------------------------------------------------------|
| dev                | Contains next major features of the project – contains multiple features bundled together. Will be merged to main. |
| main               | main branch – used to integrate changes and create releases. Locked – history cannot be overwritten                |
| maint-1.5          | maintenance branch for the projectv1.5 – bug fixes for scopy 1.5 are created here. Used to create v1.5.x releases  |
| v<x.xx.xx>[-rc<x>] | Version branch of the project. Versioned releases and release candidates. Locked.                                  |

# Review process
After creating a pull request, the developer should ask reviewers to review their code. Ideally the pull request
should contain only one feature/bug which can be easily understood by the reviewer.

When reviewing keep a few things in mind

- Does the pull request pass automated tests/ styling checks/ etc. ?
- Is the intent clear ?
- Is the solution directly addressing the problem ?
- Is the solution clear ?
- Is the code implementing the solution clear ? Can it be simplified ?
- Does the code introduce any maintenance effort ?
- Are there any side effects to the solution ?
- Can the solution be tested ?
- Can the solution be automatically tested ? Is the automatic test implemented ?
- Are code commits structured nicely ? Commits shouldn't be too big, each commit should address a single change in the code. There should be no "

