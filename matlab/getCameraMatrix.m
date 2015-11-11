function cameraMatrix = getCameraMatrix

    % Load Images
    im1 = imread('im1.jpg');
    im2 = imread('im2.jpg');

    % Load Pixel Coordinates
    load('im1_coords')
    load('im2_coords')

    % Load Points in world frame
    load('im1_world')
    load('im2_world')

    % TODO: Compute affine camera matrix from correspondences

    % TODO: Set up a matrix A from your world frame points

    % TODO: Set up a matrix b from your pixel coordinates

    % TODO: Solve a least squares problem

    % TODO: Reconstruct affine camera matrix
    % This is currently just a placeholder to get something to plot, but it is
    % completely wrong
    cameraMatrix = zeros(3, 4);
    cameraMatrix(1, 1) = 1;
    cameraMatrix(2, 2) = 1;
    cameraMatrix(3, 4) = 1;

    % Compute projections of world points into images
    im1_proj = cameraMatrix * [im1_world; ones(1, 12)];
    im2_proj = cameraMatrix * [im2_world; ones(1, 12)];

    % Plot everything
    figure(1)
    clf
    subplot(2, 1, 1)
    imshow(im1)
    hold on
    plot(im1_coords(1, :), im1_coords(2, :), 'r.', 'MarkerSize', 20)
    plot(im1_proj(1, :), im1_proj(2, :), 'bx', 'MarkerSize', 20)
    title('Image 1')

    subplot(2, 1, 2)
    imshow(im2)
    hold on
    plot(im2_coords(1, :), im2_coords(2, :), 'r.', 'MarkerSize', 20)
    plot(im2_proj(1, :), im2_proj(2, :), 'bx', 'MarkerSize', 20)
    title('Image 2')

end
